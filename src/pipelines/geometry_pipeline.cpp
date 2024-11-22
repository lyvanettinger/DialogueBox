#include "pch.hpp"

#include "resource_util.hpp"
#include "dx12_helpers.hpp"

#include "pipelines/geometry_pipeline.hpp"

#include "command_queue.hpp"
#include "renderer.hpp"
#include "camera.hpp"

using namespace Util;
using namespace Microsoft::WRL;

GeometryPipeline::GeometryPipeline(Renderer& renderer, std::shared_ptr<Camera>& camera)
    : _renderer(renderer)
    , _camera(camera)
{
    CreatePipeline();
    InitializeAssets();
}

GeometryPipeline::~GeometryPipeline()
{

}

void GeometryPipeline::PopulateCommandlist(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>& commandList)
{
    // Set necessary stuff.
    commandList->SetPipelineState(_pipelineState.Get());
    commandList->SetGraphicsRootSignature(_rootSignature.Get());

    // Start recording.
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
    commandList->IASetIndexBuffer(&_indexBufferView);
    //commandList->SetGraphicsRootDescriptorTable(1, _renderer._srvHeap->GetGPUDescriptorHandleForHeapStart());

    // Update the MVP matrix
    XMMATRIX mvpMatrix = XMMatrixMultiply(_camera->model, _camera->view);
    mvpMatrix = XMMatrixMultiply(mvpMatrix, _camera->projection);
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);

    commandList->DrawIndexedInstanced(_indexCount, 1, 0, 0, 0);
}

void GeometryPipeline::Update(float deltaTime)
{
    static double totalTime = 0.0f;
    totalTime += deltaTime;
    if (totalTime > 4.0f)
    {
        totalTime = 0.0f;
    }

    // Update the model matrix.
    float angle = static_cast<float>(totalTime * 90.0);
    const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
    _camera->model = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));

    // Update the view matrix.
    _camera->view = XMMatrixLookAtLH(_camera->position, _camera->position + _camera->front, _camera->up);

    // Update the projection matrix.
    _camera->projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(_camera->fov), _renderer._aspectRatio, 0.1f, 100.0f);
}

void GeometryPipeline::CreatePipeline()
{
    // Create an empty root signature.
    // https://www.3dgep.com/learning-directx-12-2/#Root_Signatures
    // A root signature defines the paramteres that are passed to the shader pipeline.
    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    CD3DX12_DESCRIPTOR_RANGE textureDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

    CD3DX12_ROOT_PARAMETER rootParameters[2];
    rootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[1].InitAsDescriptorTable(1, &textureDescriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_STATIC_SAMPLER_DESC albedoSampler;
    albedoSampler.Init(0);
    albedoSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &albedoSampler, rootSignatureFlags);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ThrowIfFailed(_renderer._device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&_rootSignature)));


    // Create the pipeline state, which includes compiling and loading shaders.
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    ThrowIfFailed(D3DCompileFromFile(L"assets/shaders/uber_vs.hlsl", nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
    ThrowIfFailed(D3DCompileFromFile(L"assets/shaders/uber_ps.hlsl", nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Describe and create the graphics pipeline state object (PSO).
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    pipelineStateStream.pRootSignature = _rootSignature.Get();
    pipelineStateStream.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(_renderer._device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&_pipelineState)));
}

void GeometryPipeline::InitializeAssets()
{
    auto commandList = _renderer._copyCommandQueue->GetCommandList();

    std::vector<Vertex> cubeVertices;
    std::vector<uint16_t> cubeIndices;
    CreateCube(cubeVertices, cubeIndices, 1.0f);
    
    // Create the vertex buffer.
    ComPtr<ID3D12Resource> intermediateVertexBuffer;
    LoadBufferResource(_renderer._device, commandList,
        &_vertexBuffer, &intermediateVertexBuffer,
        cubeVertices.size(), sizeof(Vertex), cubeVertices.data());

    _vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
    _vertexBufferView.StrideInBytes = sizeof(Vertex);
    _vertexBufferView.SizeInBytes = sizeof(Vertex) * cubeVertices.size();

    // Create the index buffer.
    ComPtr<ID3D12Resource> intermediateIndexBuffer;
    LoadBufferResource(_renderer._device, commandList,
        &_IndexBuffer, &intermediateIndexBuffer,
        cubeIndices.size(), sizeof(uint16_t), cubeIndices.data());
    _indexCount = static_cast<int>(cubeIndices.size());

    _indexBufferView.BufferLocation = _IndexBuffer->GetGPUVirtualAddress();
    _indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    _indexBufferView.SizeInBytes = _indexCount * sizeof(uint16_t);

    // Create the texture.
    ComPtr<ID3D12Resource> intermediateAlbedoBuffer;
    _albedoTextureHandle = _renderer._srvHeap->GetCPUDescriptorHandleForHeapStart();
    /*LoadTextureFromFile(_renderer._device, commandList,
        &_albedoTexture, &intermediateAlbedoBuffer,
        L"Utila.jpeg");*/
    _albedoTextureView.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    _albedoTextureView.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    _albedoTextureView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    _albedoTextureView.Texture2D.MipLevels = 1;
    //_renderer._device->CreateShaderResourceView(_albedoTexture.Get(), &_albedoTextureView, _albedoTextureHandle);
    //_renderer._device->CopyDescriptorsSimple(1, _renderer._srvHeap->GetCPUDescriptorHandleForHeapStart(), _albedoTextureHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Execute list
    uint64_t fenceValue = _renderer._copyCommandQueue->ExecuteCommandList(commandList);
    _renderer._copyCommandQueue->WaitForFenceValue(fenceValue);
}