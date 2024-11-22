#include "pch.hpp"
#include "resource_util.hpp"

#include "dx12_helpers.hpp"

#ifndef _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

void Util::CreateCube(
    std::vector<Vertex>& vertices,
    std::vector<uint16_t>& indices,
    float size)
{
    // Cube is centered at 0,0,0.
    float s = size * 0.5f;

    // 8 edges of cube.
    DirectX::XMFLOAT3 p[8] = { { s, s, -s }, { s, s, s },   { s, -s, s },   { s, -s, -s },
                      { -s, s, s }, { -s, s, -s }, { -s, -s, -s }, { -s, -s, s } };
    // 6 face normals
    DirectX::XMFLOAT3 n[6] = { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };
    // 4 unique texture coordinates
    DirectX::XMFLOAT2 t[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };

    // Indices for the vertex positions.
    uint16_t i[24] = {
        0, 1, 2, 3,  // +X
        4, 5, 6, 7,  // -X
        4, 1, 0, 5,  // +Y
        2, 7, 6, 3,  // -Y
        1, 4, 7, 2,  // +Z
        5, 0, 3, 6   // -Z
    };

    for (uint16_t f = 0; f < 6; ++f)  // For each face of the cube.
    {
        // Four vertices per face.
        vertices.emplace_back(p[i[f * 4 + 0]], n[f], t[0]);
        vertices.emplace_back(p[i[f * 4 + 1]], n[f], t[1]);
        vertices.emplace_back(p[i[f * 4 + 2]], n[f], t[2]);
        vertices.emplace_back(p[i[f * 4 + 3]], n[f], t[3]);

        // First triangle.
        indices.emplace_back(f * 4 + 0);
        indices.emplace_back(f * 4 + 1);
        indices.emplace_back(f * 4 + 2);

        // Second triangle
        indices.emplace_back(f * 4 + 2);
        indices.emplace_back(f * 4 + 3);
        indices.emplace_back(f * 4 + 0);
    }
}

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
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
    ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
    const std::wstring& fileName)
{
    fs::path filePath(fileName);
    if (!fs::exists(filePath))
    {
        throw std::exception("File not found.");
    }

    /*DirectX::TexMetadata metadata;
    DirectX::ScratchImage scratchImage;*/

    /*if (filePath.extension() == ".dds")
    {
        ThrowIfFailed(DirectX::LoadFromDDSFile(
            fileName.c_str(),
            DirectX::DDS_FLAGS_NONE,
            &metadata,
            scratchImage));
    }
    else if (filePath.extension() == ".hdr")
    {
        ThrowIfFailed(DirectX::LoadFromHDRFile(
            fileName.c_str(),
            &metadata,
            scratchImage));
    }
    else if (filePath.extension() == ".tga")
    {
        ThrowIfFailed(DirectX::LoadFromTGAFile(
            fileName.c_str(),
            &metadata,
            scratchImage));
    }
    else
    {
        ThrowIfFailed(DirectX::LoadFromWICFile(
            fileName.c_str(),
            DirectX::WIC_FLAGS_NONE,
            &metadata,
            scratchImage));
    }*/

    D3D12_RESOURCE_DESC textureDesc = {};
    /*switch (metadata.dimension)
    {
    case DirectX::TEX_DIMENSION_TEXTURE1D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(
            metadata.format,
            static_cast<UINT64>(metadata.width),
            static_cast<UINT16>(metadata.arraySize));
        break;
    case DirectX::TEX_DIMENSION_TEXTURE2D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            metadata.format,
            static_cast<UINT64>(metadata.width),
            static_cast<UINT>(metadata.height),
            static_cast<UINT16>(metadata.arraySize));
        break;
    case DirectX::TEX_DIMENSION_TEXTURE3D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(
            metadata.format,
            static_cast<UINT64>(metadata.width),
            static_cast<UINT>(metadata.height),
            static_cast<UINT16>(metadata.depth));
        break;
    default:
        throw std::exception("Invalid texture dimension.");
        break;
    }*/
    
    CD3DX12_HEAP_PROPERTIES textureHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(device->CreateCommittedResource(
        &textureHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(pDestinationResource)));

    /*std::vector<D3D12_SUBRESOURCE_DATA> subresources(scratchImage.GetImageCount());
    const DirectX::Image* pImages = scratchImage.GetImages();
    for (int i = 0; i < scratchImage.GetImageCount(); ++i)
    {
        auto& subresource = subresources[i];
        subresource.RowPitch = pImages[i].rowPitch;
        subresource.SlicePitch = pImages[i].slicePitch;
        subresource.pData = pImages[i].pixels;
    }*/

    if (pDestinationResource)
    {
        TransitionResource(commandList, *pDestinationResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

        /*UINT64 requiredSize = GetRequiredIntermediateSize(*pDestinationResource, 0, static_cast<uint32_t>(subresources.size()));

        D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(requiredSize);
        CD3DX12_HEAP_PROPERTIES bufferHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        ThrowIfFailed(device->CreateCommittedResource(
            &bufferHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(pIntermediateResource)
        ));*/
        //UpdateSubresources(commandList.Get(), *pDestinationResource, *pIntermediateResource, 0, 0, static_cast<UINT>(subresources.size()), subresources.data());
    }
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