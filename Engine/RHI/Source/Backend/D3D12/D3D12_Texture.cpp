#include "RHI/Backend/D3D12/D3D12_Texture.h"
#include <cstring>

using Microsoft::WRL::ComPtr;

namespace grom {

D3D12Texture::~D3D12Texture()
{
}

void* D3D12Texture::GetHandle() { return m_Resource.Get(); }
TextureDesc& D3D12Texture::GetDesc() { return m_Desc; }

void D3D12Texture::Upload(void* data, u32 mipLevel)
{
    // Stub implementation - would need command list from device
    (void)data; (void)mipLevel;
}

static DXGI_FORMAT ConvertFormat(EFormat fmt)
{
    switch (fmt) {
    case EFormat::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
    case EFormat::R8G8B8A8_SRGB:  return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case EFormat::B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
    case EFormat::R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case EFormat::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case EFormat::D32_FLOAT:      return DXGI_FORMAT_D32_FLOAT;
    case EFormat::R32_FLOAT:      return DXGI_FORMAT_R32_FLOAT;
    case EFormat::R8_UNORM:       return DXGI_FORMAT_R8_UNORM;
    default: return DXGI_FORMAT_UNKNOWN;
    }
}

D3D12Texture* D3D12Texture::Create(TextureDesc& desc, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    DXGI_FORMAT dxgiFormat = ConvertFormat(desc.Format);
    if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
        return nullptr;

    D3D12Texture* texture = new D3D12Texture();
    texture->m_Desc = desc;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resDesc.Width = desc.Width;
    resDesc.Height = desc.Height;
    resDesc.DepthOrArraySize = static_cast<UINT16>(desc.ArraySize);
    resDesc.MipLevels = static_cast<UINT16>(desc.MipLevels);
    resDesc.Format = dxgiFormat;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

    if (desc.IsRenderTarget) {
        flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }
    if (desc.IsDepthStencil) {
        flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    }
    if (desc.IsUnorderedAccess) {
        flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    resDesc.Flags = flags;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_CLEAR_VALUE* clearValue = nullptr;
    D3D12_CLEAR_VALUE depthClear = {};
    if (desc.IsDepthStencil) {
        depthClear.Format = dxgiFormat;
        depthClear.DepthStencil.Depth = 1.0f;
        depthClear.DepthStencil.Stencil = 0;
        clearValue = &depthClear;
    }

    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        initialState,
        clearValue,
        IID_PPV_ARGS(&texture->m_Resource));

    if (FAILED(hr)) {
        delete texture;
        return nullptr;
    }

    // Create upload buffer for initial data
    if (desc.InitialData) {
        D3D12_RESOURCE_DESC uploadDesc = resDesc;
        uploadDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        UINT64 uploadSize = 0;
        device->GetCopyableFootprints(&resDesc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadSize);

        D3D12_RESOURCE_DESC uploadResDesc = {};
        uploadResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        uploadResDesc.Width = uploadSize;
        uploadResDesc.Height = 1;
        uploadResDesc.DepthOrArraySize = 1;
        uploadResDesc.MipLevels = 1;
        uploadResDesc.Format = DXGI_FORMAT_UNKNOWN;
        uploadResDesc.SampleDesc.Count = 1;
        uploadResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        D3D12_HEAP_PROPERTIES uploadHeapProps = {};
        uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

        hr = device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &uploadResDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&texture->m_UploadBuffer));

        if (FAILED(hr)) {
            delete texture;
            return nullptr;
        }

        // Copy initial data
        void* mapped = nullptr;
        D3D12_RANGE readRange = { 0, 0 };
        texture->m_UploadBuffer->Map(0, &readRange, &mapped);
        std::memcpy(mapped, desc.InitialData, static_cast<size_t>(uploadSize));
        texture->m_UploadBuffer->Unmap(0, nullptr);

        // Copy from upload to texture
        D3D12_TEXTURE_COPY_LOCATION dst = {};
        dst.pResource = texture->m_Resource.Get();
        dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dst.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION src = {};
        src.pResource = texture->m_UploadBuffer.Get();
        src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        src.PlacedFootprint.Offset = 0;
        src.PlacedFootprint.Footprint.Format = dxgiFormat;
        src.PlacedFootprint.Footprint.Width = desc.Width;
        src.PlacedFootprint.Footprint.Height = desc.Height;
        src.PlacedFootprint.Footprint.Depth = 1;
        src.PlacedFootprint.Footprint.RowPitch = (desc.Width * 4 + 255) & ~255;

        cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = texture->m_Resource.Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        cmdList->ResourceBarrier(1, &barrier);
    }

    return texture;
}

} // namespace grom