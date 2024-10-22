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
	void Render();

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
    uint64_t _fenceValues[FRAME_COUNT] = {};
    const float clearColor[4] = { 255.0f / 255.0f, 182.0f / 255.0f, 193.0f / 255.0f, 1.0f }; // pink :)
    bool _useWarpDevice;

    void InitializeGraphics();
    void CreateDepthBuffer();

    // friend classes
    friend class GeometryPipeline;
    friend class UIPipeline;
};