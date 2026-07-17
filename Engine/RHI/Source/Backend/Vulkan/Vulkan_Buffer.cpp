#include "RHI/Backend/Vulkan/Vulkan_Buffer.h"
#include <cstring>

namespace grom {

VulkanBuffer::~VulkanBuffer()
{
    if (m_Buffer && m_Allocator) {
        vmaDestroyBuffer(m_Allocator, m_Buffer, m_Allocation);
    }
}

void* VulkanBuffer::GetHandle() { return m_Buffer; }
BufferDesc& VulkanBuffer::GetDesc() { return m_Desc; }

void VulkanBuffer::Update(void* data, u32 size, u32 offset)
{
    if (m_MappedData) {
        std::memcpy(static_cast<u8*>(m_MappedData) + offset, data, size);
    }
}

void* VulkanBuffer::Map()
{
    return m_MappedData;
}

void VulkanBuffer::Unmap()
{
    // For VMA with HOST_VISIBLE, data is immediately available
}

VulkanBuffer* VulkanBuffer::Create(BufferDesc& desc, VkDevice /*device*/, VmaAllocator allocator)
{
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_AUTO;
    VmaAllocationCreateFlags allocFlags = 0;

    switch (desc.Type)
    {
    case EBufferType::Vertex:
        usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case EBufferType::Index:
        usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case EBufferType::Constant:
        usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        allocFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        break;
    case EBufferType::Structured:
        usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        break;
    }

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = desc.Size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = memUsage;
    allocCreateInfo.flags = allocFlags;

    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocResultInfo;

    VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocResultInfo);
    if (result != VK_SUCCESS)
        return nullptr;

    VulkanBuffer* buf = new VulkanBuffer();
    buf->m_Buffer = buffer;
    buf->m_Allocation = allocation;
    buf->m_Allocator = allocator;
    buf->m_Desc = desc;
    buf->m_MappedData = allocResultInfo.pMappedData;

    if (desc.InitialData && desc.Size > 0 && allocResultInfo.pMappedData) {
        std::memcpy(allocResultInfo.pMappedData, desc.InitialData, desc.Size);
    }

    return buf;
}

} // namespace grom