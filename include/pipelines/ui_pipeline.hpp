#pragma once

class Renderer;

class UIPipeline
{
public:
	UIPipeline(Renderer& renderer);
	~UIPipeline();

	ID3D12CommandList* PopulateCommandlist();
private:
	Renderer& _renderer;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;

	void CreatePipeline();
};