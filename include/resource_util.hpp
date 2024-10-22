#pragma once

namespace Util
{
	void LoadBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
		ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, 
		size_t numElements, size_t elementSize, const void* bufferData, 
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	
	
	void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, 
		Microsoft::WRL::ComPtr<ID3D12Resource> resource, 
		D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

	struct CommandResource
	{
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList;
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	};
}