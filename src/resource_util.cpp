#include "pch.hpp"
#include "resource_util.hpp"

#include "dx12_helpers.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

// #include <DirectXTex.h> // can add later to load DDS textures

void Util::LoadBufferResource(
    Microsoft::WRL::ComPtr<ID3D12Device> device,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
	ID3D12Resource** pDestinationResource,
	ID3D12Resource** pIntermediateResource,
	size_t numElements, size_t elementSize,
	const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
    size_t bufferSize = numElements * elementSize;

    {   // Create a committed resource for the GPU resource in a default heap.
        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);
        ThrowIfFailed(device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(pDestinationResource)));
    }

    // Create an committed resource for the upload.
    if (bufferData)
    {
        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
        ThrowIfFailed(device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(pIntermediateResource)));

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.Get(),
            *pDestinationResource, *pIntermediateResource,
            0, 0, 1, &subresourceData);
    }
}

void Util::LoadTextureFromFile(
    Microsoft::WRL::ComPtr<ID3D12Device> device,
    ID3D12Resource** pDestinationResource, const std::string& filePath,
    DXGI_FORMAT format, Util::TextureDimension dimension)
{
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    float* data = stbi_loadf(filePath.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        CD3DX12_RESOURCE_DESC textureDesc = {};
        switch (dimension)
        {
        case Util::TextureDimension::eTEX1D:
            // not yet implemented
            assert(false);
            //textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(format, width);
            break;
        case Util::TextureDimension::eTEX2D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height);
            break;
        case Util::TextureDimension::eTEX3D:
            // not yet implemented
            assert(false);
            //textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(format, width, height)
            break;
        default:
            assert(false);
        }

        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
        ThrowIfFailed(device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(pDestinationResource)));

        D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvHandle = {}; // TODO: pass this back??
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = format;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        device->CreateShaderResourceView(*pDestinationResource, &srvDesc, cbvSrvHandle);
    }
    else
    {
        assert(false);
    }
    stbi_image_free(data);
}

void Util::TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
    Microsoft::WRL::ComPtr<ID3D12Resource> resource,
    D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource.Get(),
        beforeState, afterState);

    commandList->ResourceBarrier(1, &barrier);
}