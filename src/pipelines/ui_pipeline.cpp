#include "pch.hpp"

#include "pipelines/ui_pipeline.hpp"

#include "dx12_helpers.hpp"
#include "renderer.hpp"

UIPipeline::UIPipeline(Renderer& renderer) :
	_renderer(renderer)
{
	CreatePipeline();
}

UIPipeline::~UIPipeline()
{

}

void UIPipeline::PopulateCommandlist()
{

}

void UIPipeline::CreatePipeline()
{

}
