#pragma once
#include "../../RHI_Buffer.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <vulkan/vulkan.h>
#include "vk_mem_alloc_wrapper.h"

namespace grom {

class VulkanBuffer : public Buffer
{
public:
    VulkanBuffer() = default;
    ~VulkanBuffer() override;

    void* GetHandle() override;
    BufferDesc& GetDesc() override;
    void Update(void* data, u32 size, u32 offset) override;
    void* Map() override;
    void Unmap() override;

    VkBuffer GetVkBuffer() const { return m_Buffer; }
    VmaAllocation GetAllocation() const { return m_Allocation; }

    static VulkanBuffer* Create(BufferDesc& desc, VkDevice device, VmaAllocator allocator);

private:
    VkBuffer m_Buffer = VK_NULL_HANDLE;
    VmaAllocation m_Allocation = VK_NULL_HANDLE;
    BufferDesc m_Desc;
    void* m_MappedData = nullptr;
    VkDeviceSize m_AlignedSize = 0;
    VmaAllocator m_Allocator = VK_NULL_HANDLE;
};

} // namespace grom
