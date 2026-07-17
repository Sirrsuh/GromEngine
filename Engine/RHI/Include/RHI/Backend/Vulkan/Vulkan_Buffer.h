#pragma once
#include "../../RHI_Buffer.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <vulkan/vulkan.h>

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
    VkDeviceMemory GetMemory() const { return m_Memory; }

    static VulkanBuffer* Create(BufferDesc& desc, VkDevice device, VkPhysicalDevice physDevice);

private:
    VkBuffer m_Buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_Memory = VK_NULL_HANDLE;
    BufferDesc m_Desc;
    void* m_MappedData = nullptr;
    VkDeviceSize m_AlignedSize = 0;
    VkDevice m_Device = VK_NULL_HANDLE;
};

} // namespace grom
