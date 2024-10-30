#pragma once

namespace Util
{
	void LoadBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, 
		size_t numElements, size_t elementSize, const void* bufferData, 
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	void LoadTextureFromFile(Microsoft::WRL::ComPtr<ID3D12Device> device, 
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
		const std::wstring& filePath);
	
	
	void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, 
		Microsoft::WRL::ComPtr<ID3D12Resource> resource, 
		D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
}