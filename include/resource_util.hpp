#pragma once

namespace Util
{
	void LoadBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device,Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
}