#define VK_USE_PLATFORM_WIN32_KHR
#include "RHI/Backend/Vulkan/Vulkan_Buffer.h"
#include <cstring>

namespace grom {

VulkanBuffer::~VulkanBuffer()
{
    if (m_Buffer) vkDestroyBuffer(m_Device, m_Buffer, nullptr);
    if (m_Memory) vkFreeMemory(m_Device, m_Memory, nullptr);
}

void* VulkanBuffer::GetHandle() { return m_Buffer; }
BufferDesc& VulkanBuffer::GetDesc() { return m_Desc; }

void VulkanBuffer::Update(void* data, u32 size, u32 offset)
{
    if (m_MappedData)
    {
        memcpy(static_cast<u8*>(m_MappedData) + offset, data, size);
    }
}

void* VulkanBuffer::Map()
{
    return m_MappedData;
}

void VulkanBuffer::Unmap() {}

VulkanBuffer* VulkanBuffer::Create(BufferDesc& desc, VkDevice device, VkPhysicalDevice physDevice)
{
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkSharingMode sharing = VK_SHARING_MODE_EXCLUSIVE;

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
        break;
    case EBufferType::Structured:
        usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        break;
    }

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = desc.Size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = sharing;

    VkBuffer buffer;
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        return nullptr;

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, buffer, &memReqs);

    VkPhysicalDeviceMemoryProperties physMemProps;
    vkGetPhysicalDeviceMemoryProperties(physDevice, &physMemProps);

    u32 memoryType = UINT32_MAX;
    for (u32 i = 0; i < physMemProps.memoryTypeCount; ++i)
    {
        if ((memReqs.memoryTypeBits & (1 << i)) &&
            (physMemProps.memoryTypes[i].propertyFlags & memProps) == memProps)
        {
            memoryType = i;
            break;
        }
    }

    if (memoryType == UINT32_MAX)
    {
        vkDestroyBuffer(device, buffer, nullptr);
        return nullptr;
    }

VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = memoryType;

    VkDeviceMemory memory;
    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
    {
        vkDestroyBuffer(device, buffer, nullptr);
        return nullptr;
    }

    vkBindBufferMemory(device, buffer, memory, 0);

    VulkanBuffer* buf = new VulkanBuffer();
    buf->m_Device = device;
    buf->m_Buffer = buffer;
    buf->m_Memory = memory;
    buf->m_Desc = desc;

    VkResult res = vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, &buf->m_MappedData);
    if (res == VK_SUCCESS && desc.InitialData && desc.Size > 0)
    {
        memcpy(buf->m_MappedData, desc.InitialData, desc.Size);
    }
    else
    {
        buf->m_MappedData = nullptr;
    }

    return buf;
}

} // namespace grom
