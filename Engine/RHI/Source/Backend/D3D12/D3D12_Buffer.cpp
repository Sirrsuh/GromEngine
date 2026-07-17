#include "RHI/Backend/D3D12/D3D12_Buffer.h"
#include <cstring>

using Microsoft::WRL::ComPtr;

namespace grom {

D3D12Buffer::~D3D12Buffer()
{
}

void* D3D12Buffer::GetHandle() { return m_Resource.Get(); }
BufferDesc& D3D12Buffer::GetDesc() { return m_Desc; }

void D3D12Buffer::Update(void* data, u32 size, u32 offset)
{
    if (m_MappedData) {
        std::memcpy(static_cast<u8*>(m_MappedData) + offset, data, size);
    }
}

void* D3D12Buffer::Map()
{
    return m_MappedData;
}

void D3D12Buffer::Unmap()
{
    // For upload heaps, data is available immediately
}

D3D12Buffer* D3D12Buffer::Create(BufferDesc& desc, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    D3D12Buffer* buffer = new D3D12Buffer();
    buffer->m_Desc = desc;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = desc.Size;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    if (desc.Type == EBufferType::Structured || desc.Type == EBufferType::Constant) {
        resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&buffer->m_Resource));

    if (FAILED(hr)) {
        delete buffer;
        return nullptr;
    }

    // Create upload buffer for initialization
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&buffer->m_UploadBuffer));

    if (FAILED(hr)) {
        delete buffer;
        return nullptr;
    }

    // Copy data if provided
    if (desc.InitialData && desc.Size > 0) {
        void* mappedData = nullptr;
        D3D12_RANGE readRange = { 0, 0 };
        buffer->m_UploadBuffer->Map(0, &readRange, &mappedData);
        std::memcpy(mappedData, desc.InitialData, desc.Size);
        buffer->m_UploadBuffer->Unmap(0, nullptr);

        // Schedule copy from upload to default buffer
        if (cmdList) {
            cmdList->CopyBufferRegion(buffer->m_Resource.Get(), 0, buffer->m_UploadBuffer.Get(), 0, desc.Size);

            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = buffer->m_Resource.Get();
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            cmdList->ResourceBarrier(1, &barrier);
        }
    }

    // Map for constant buffers
    if (desc.Type == EBufferType::Constant) {
        D3D12_RANGE readRange = { 0, 0 };
        buffer->m_Resource->Map(0, &readRange, &buffer->m_MappedData);
    }

    return buffer;
}

} // namespace grom