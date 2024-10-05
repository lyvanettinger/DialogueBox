#pragma once

class Application;
class GeometryPipeline;
class UIPipeline;

class Renderer
{
public:
	Renderer(std::shared_ptr<Application> app);
	~Renderer();

	void Render();

private:
    std::shared_ptr<Application> _app;
    std::unique_ptr<GeometryPipeline> _geometryPipeline;
    std::unique_ptr<UIPipeline> _uiPipeline;

    UINT _width;
    UINT _height;
    float _aspectRatio;

    // Pipeline objects
    CD3DX12_VIEWPORT _viewport;
    CD3DX12_RECT _scissorRect;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> _swapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> _device;
    Microsoft::WRL::ComPtr<ID3D12Resource> _renderTargets[FRAME_COUNT];
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _commandAllocator;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> _commandQueue;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _rtvHeap;
    UINT _rtvDescriptorSize;

    // Synchronization objects
    UINT _frameIndex;
    Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
    HANDLE _fenceEvent;
    UINT64 _fenceValue;

    bool _useWarpDevice;

    void LoadPipeline();
    void LoadAssets();
    void WaitForPreviousFrame();

    // friend classes
    friend class GeometryPipeline;
};