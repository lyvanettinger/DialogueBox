#pragma once

class Application;
class GeometryPipeline;
class UIPipeline;
class CommandQueue;
struct Camera;

class Renderer
{
public:
	Renderer(std::shared_ptr<Application> app);
	~Renderer();

    void Update(float deltaTime);
	void Render(float deltaTime);

    void ResizeWindow(UINT width, UINT height);

    void Flush();

private:
    std::shared_ptr<Application> _app;
    std::shared_ptr<Camera> _camera;

    std::unique_ptr<GeometryPipeline> _geometryPipeline;
    std::unique_ptr<UIPipeline> _uiPipeline;

    UINT _width;
    UINT _height;
    float _aspectRatio;

    CD3DX12_VIEWPORT _viewport;
    CD3DX12_RECT _scissorRect;

    Microsoft::WRL::ComPtr<IDXGISwapChain3> _swapChain;
    Microsoft::WRL::ComPtr<ID3D12Device2> _device;

    std::unique_ptr<CommandQueue> _directCommandQueue;
    std::unique_ptr<CommandQueue> _copyCommandQueue;

    Microsoft::WRL::ComPtr<ID3D12Resource> _renderTargets[FRAME_COUNT];
    Microsoft::WRL::ComPtr<ID3D12Resource> _depthBuffer;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _rtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _dsvHeap;
    UINT _rtvDescriptorSize;

    UINT _frameIndex;
    bool _useWarpDevice;

    void InitializeGraphics();
    void ResizeDepthBuffer();

    // friend classes
    friend class GeometryPipeline;
};