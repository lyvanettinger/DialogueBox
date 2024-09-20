#pragma once
#include "pch.hpp"

class Application;

class Renderer
{
public:
	Renderer(std::shared_ptr<Application> app);
	~Renderer();

	void Render();

private:
    std::shared_ptr<Application> _app;

    UINT _width;
    UINT _height;
    float _aspectRatio;

    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
    };

    // Pipeline objects
    CD3DX12_VIEWPORT _viewport;
    CD3DX12_RECT _scissorRect;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> _swapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> _device;
    Microsoft::WRL::ComPtr<ID3D12Resource> _renderTargets[FRAME_COUNT];
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _commandAllocator;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> _commandQueue;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _rtvHeap;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;
    UINT _rtvDescriptorSize;

    // App resources.
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    // Synchronization objects
    UINT _frameIndex;
    Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
    HANDLE _fenceEvent;
    UINT64 _fenceValue;

    bool _useWarpDevice;

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();
};