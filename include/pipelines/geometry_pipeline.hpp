#pragma once

class Renderer;

class GeometryPipeline
{
public:
	GeometryPipeline(Renderer& renderer);
	~GeometryPipeline();

	void LoadAssets();

	ID3D12CommandList* PopulateCommandlist();
private:
	Renderer& _renderer;

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

	Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;

	Microsoft::WRL::ComPtr<ID3D12Resource> _vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
};