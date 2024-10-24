#pragma once

namespace Util
{
	enum class TextureDimension
	{
		eTEX1D = 0,
		eTEX2D,
		eTEX3D
	};

	void LoadBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, 
		size_t numElements, size_t elementSize, const void* bufferData, 
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	void LoadTextureFromFile(Microsoft::WRL::ComPtr<ID3D12Device> device,
		ID3D12Resource** pDestinationResource, const std::string& fileName,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, TextureDimension dimension = TextureDimension::eTEX2D);
	
	
	void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, 
		Microsoft::WRL::ComPtr<ID3D12Resource> resource, 
		D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
}