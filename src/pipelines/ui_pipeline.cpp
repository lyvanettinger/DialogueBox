#include "pch.hpp"

#include "pipelines/ui_pipeline.hpp"

#include "dx12_helpers.hpp"
#include "renderer.hpp"

UIPipeline::UIPipeline(Renderer& renderer) :
	_renderer(renderer)
{

}

UIPipeline::~UIPipeline()
{

}

ID3D12CommandList* UIPipeline::PopulateCommandlist()
{
	return _commandList.Get();
}

void UIPipeline::LoadAssets()
{

}
