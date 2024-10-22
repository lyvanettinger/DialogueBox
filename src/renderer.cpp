/*
*   Code written using these tutorials:
*     - https://learn.microsoft.com/en-us/windows/win32/direct3d12/direct3d-12-graphics
*     - https://www.3dgep.com/category/graphics-programming/directx/
*/

#include "pch.hpp"

#include "renderer.hpp"

#include "dx12_helpers.hpp"
#include "resource_util.hpp"
#include "glfw_app.hpp"
#include "command_queue.hpp"

#include "camera.hpp"

#include "pipelines/geometry_pipeline.hpp"
#include "pipelines/ui_pipeline.hpp"


Renderer::Renderer(std::shared_ptr<Application> app) :
	_app(app),
    _width(_app->GetWidth()),
    _height(_app->GetHeight()),
	_viewport(0.0f, 0.0f, static_cast<float>(_width), static_cast<float>(_height)),
	_scissorRect(0, 0, static_cast<LONG>(_width), static_cast<LONG>(_height)),
	_rtvDescriptorSize(0),
    _useWarpDevice(false)
{
    _aspectRatio = static_cast<float>(_width) / static_cast<float>(_height);
    _camera = std::make_shared<Camera>();

    InitializeGraphics();

    // Create pipelines
    _geometryPipeline = std::make_unique<GeometryPipeline>(*this, _camera);
    _uiPipeline = std::make_unique<UIPipeline>(*this);

    Flush();
}

Renderer::~Renderer()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    Flush();
}

void Renderer::Update(float deltaTime)
{
    _geometryPipeline->Update(deltaTime);
}

void Renderer::Render()
{
    auto commandList = _directCommandQueue->GetCommandList();

    // Clear targets.
    Util::TransitionResource(commandList, _renderTargets[_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvHeap->GetCPUDescriptorHandleForHeapStart(), _frameIndex, _rtvDescriptorSize);
    auto dsvHandle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Record command lists.
    _geometryPipeline->PopulateCommandlist(commandList);

    // Sync up resource(s) (might need this inbetween some stages later)
    Util::TransitionResource(commandList, _renderTargets[_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    // Execute commandlist.
    uint64_t fenceValue = _directCommandQueue->ExecuteCommandList(commandList);
    _fenceValues[_frameIndex] = fenceValue;

    // Present the frame.
    Util::ThrowIfFailed(_swapChain->Present(1, 0));

    // Wait for new back buffer to be done.
    _frameIndex = _swapChain->GetCurrentBackBufferIndex();
    _directCommandQueue->WaitForFenceValue(_fenceValues[_frameIndex]);
}

void Renderer::Flush()
{
    _directCommandQueue->Flush();
    _copyCommandQueue->Flush();
}

void Renderer::InitializeGraphics()
{
    UINT dxgiFactoryFlags = 0;

    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
#if defined(_DEBUG)
    {
        Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    // Create Device.
    // https://www.3dgep.com/learning-directx-12-1/#Create_the_DirectX_12_Device
    // The DirectX 12 device is used to create resources (such as textures and buffers,
    // command lists, command queues, fences, heaps, etc…). It's not directly used for issuing draw or dispatch commands.
    // It can be considered a memory context that tracks allocations in GPU memory.
    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    Util::ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    if (_useWarpDevice)
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
        Util::ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        Util::ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&_device)
        ));
    }
    else
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
        Util::GetHardwareAdapter(factory.Get(), &hardwareAdapter, false); // bool: request for high performance adapter or not?

        Util::ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&_device)
        ));
    }

    // Create command queues
    _directCommandQueue = std::make_unique<CommandQueue>(_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
    _copyCommandQueue = std::make_unique<CommandQueue>(_device, D3D12_COMMAND_LIST_TYPE_COPY);

    // Describe and create the swap chain.
    // https://www.3dgep.com/learning-directx-12-1/#Create_the_Swap_Chain
    // The primary purpose of the swap chain is to present the rendered image to the screen.
    // Back buffer: currently being rendered to
    // Front buffer: currently being presented
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FRAME_COUNT;
    swapChainDesc.Width = _app->GetWidth();
    swapChainDesc.Height = _app->GetHeight();
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
    Util::ThrowIfFailed(factory->CreateSwapChainForHwnd(
        _directCommandQueue->GetCommandQueue().Get(),        // Swap chain needs the queue so that it can force a flush on it.
        _app->GetHWND(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    ));

    // This sample does not support fullscreen transitions.
    Util::ThrowIfFailed(factory->MakeWindowAssociation(_app->GetHWND(), DXGI_MWA_NO_ALT_ENTER));

    Util::ThrowIfFailed(swapChain.As(&_swapChain));
    _frameIndex = _swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    // https://www.3dgep.com/learning-directx-12-1/#Create_a_Descriptor_Heap
    // Descriptor heap can be considered an array of resource views such as:
    // RTV (Render Target View), SRV (Shader Resource View), UAV (Unorderd Access Views) or CBV (Constant Buffer Views)
    {
        // Describe and create a render target view (RTV) descriptor heap.
        // https://www.3dgep.com/learning-directx-12-1/#Create_the_Render_Target_Views
        // RTV (Render Target View) describes a resource that receives the final color computed by the pixel shader stage
        // and can be attached to a bind slot of the output merger stage
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FRAME_COUNT;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        Util::ThrowIfFailed(_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&_rtvHeap)));

        _rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Create the descriptor heap for the depth-stencil view (DSV).
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        Util::ThrowIfFailed(_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&_dsvHeap)));
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < FRAME_COUNT; n++)
        {
            Util::ThrowIfFailed(_swapChain->GetBuffer(n, IID_PPV_ARGS(&_renderTargets[n])));
            _device->CreateRenderTargetView(_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, _rtvDescriptorSize);

        }
    }

    // Create DSV
    D3D12_CLEAR_VALUE optimizedClearValue = {};
    optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    optimizedClearValue.DepthStencil = { 1.0f, 0 };

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, _width, _height,
        1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    Util::ThrowIfFailed(_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &optimizedClearValue,
        IID_PPV_ARGS(&_depthBuffer)
    ));

    // Update the depth-stencil view.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
    dsv.Format = DXGI_FORMAT_D32_FLOAT;
    dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsv.Texture2D.MipSlice = 0;
    dsv.Flags = D3D12_DSV_FLAG_NONE;

    _device->CreateDepthStencilView(_depthBuffer.Get(), &dsv,
        _dsvHeap->GetCPUDescriptorHandleForHeapStart());
}