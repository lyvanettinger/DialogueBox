#pragma once

class Renderer;
struct Camera;

class GeometryPipeline
{
public:
	GeometryPipeline(Renderer& renderer, std::shared_ptr<Camera>& camera);
	~GeometryPipeline();

	ID3D12CommandList* PopulateCommandlist();

	void Update(float deltaTime);
private:
	Renderer& _renderer;
	std::shared_ptr<Camera> _camera;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;

	// Temporarily just store these here. Usually these should be part of a model resource
	Microsoft::WRL::ComPtr<ID3D12Resource> _vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
	Microsoft::WRL::ComPtr<ID3D12Resource> _IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW _IndexBufferView;

	void CreatePipeline();
	void InitializeAssets();
};