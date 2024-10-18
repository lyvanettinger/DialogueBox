#include "pch.hpp"

#include "pipelines/geometry_pipeline.hpp"

#include "dx12_helpers.hpp"
#include "resource_util.hpp"

#include "renderer.hpp"
#include "camera.hpp"

struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

static Vertex cubeVertices[8] = {
    { { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } }, // 0
    { { -1.0f,  1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // 1
    { { 1.0f,  1.0f, -1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } }, // 2
    { { 1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }, // 3
    { { -1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }, // 4
    { { -1.0f,  1.0f,  1.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } }, // 5
    { { 1.0f,  1.0f,  1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 6
    { { 1.0f, -1.0f,  1.0f }, { 1.0f, 0.0f, 1.0f, 1.0f } }  // 7
};

static WORD cubeIndices[36] =
{
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};

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

ID3D12CommandList* GeometryPipeline::PopulateCommandlist()
{
    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(_commandList->Reset(_renderer._directCommandAllocator.Get(), _pipelineState.Get()));

    // Set necessary state.
    _commandList->SetGraphicsRootSignature(_rootSignature.Get());
    _commandList->RSSetViewports(1, &_renderer._viewport);
    _commandList->RSSetScissorRects(1, &_renderer._scissorRect);

    // Indicate that the back buffer will be used as a render target.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(_renderer._renderTargets[_renderer._frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    _commandList->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_renderer._rtvHeap->GetCPUDescriptorHandleForHeapStart(), _renderer._frameIndex, _renderer._rtvDescriptorSize);
    auto dsvHandle = _renderer._dsvHeap->GetCPUDescriptorHandleForHeapStart();
    _commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // Clear RTV and DSV
    // TODO: this should be moved to renderer
    const float clearColor[] = { 255.0f / 255.0f, 182.0f / 255.0f, 193.0f / 255.0f, 1.0f }; // pink :)
    _commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    _commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Start recording
    _commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
    _commandList->IASetIndexBuffer(&_IndexBufferView);
    _commandList->DrawIndexedInstanced(_countof(cubeIndices), 1, 0, 0, 0);

    // Indicate that the back buffer will now be used to present.
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(_renderer._renderTargets[_renderer._frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    _commandList->ResourceBarrier(1, &barrier);

    ThrowIfFailed(_commandList->Close());

    return _commandList.Get();
}

void GeometryPipeline::Update(float deltaTime)
{
    static double totalTime = 0.0f;
    if (totalTime > 1.0f)
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
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_ROOT_PARAMETER rootParameters[1];
    rootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

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
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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

    // Create the command list.
    ThrowIfFailed(_renderer._device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _renderer._directCommandAllocator.Get(), _pipelineState.Get(), IID_PPV_ARGS(&_commandList)));

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    ThrowIfFailed(_commandList->Close());
}

void GeometryPipeline::InitializeAssets()
{
    ComPtr<ID3D12GraphicsCommandList> copyCommandList;
    ThrowIfFailed(_renderer._device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, _renderer._copyCommandAllocator.Get(), _pipelineState.Get(), IID_PPV_ARGS(&copyCommandList)));

    // Create the vertex buffer.
    ComPtr<ID3D12Resource> intermediateVertexBuffer;
    LoadBufferResource(_renderer._device, copyCommandList,
        &_vertexBuffer, &intermediateVertexBuffer,
        _countof(cubeVertices), sizeof(Vertex), cubeVertices);

    _vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
    _vertexBufferView.StrideInBytes = sizeof(Vertex);
    _vertexBufferView.SizeInBytes = sizeof(cubeVertices);

    // Create the index buffer.
    ComPtr<ID3D12Resource> intermediateIndexBuffer;
    LoadBufferResource(_renderer._device, copyCommandList,
        &_IndexBuffer, &intermediateIndexBuffer,
        _countof(cubeIndices), sizeof(WORD), cubeIndices);

    _IndexBufferView.BufferLocation = _IndexBuffer->GetGPUVirtualAddress();
    _IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    _IndexBufferView.SizeInBytes = sizeof(cubeIndices);

    ThrowIfFailed(copyCommandList->Close());
    ID3D12CommandList* const ppCopyCommandList[] = {
        copyCommandList.Get()
    };
    _renderer._copyCommandQueue->ExecuteCommandLists(1, ppCopyCommandList); // TODO: This should probably be done by the renderer
}