#pragma once

class Renderer;

class UIPipeline
{
public:
	UIPipeline(Renderer& renderer);
	~UIPipeline();

	void PopulateCommandlist();
private:
	Renderer& _renderer;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;

	void CreatePipeline();
};