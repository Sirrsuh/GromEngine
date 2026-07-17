#pragma once
#include "../../RHI_Texture.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <vulkan/vulkan.h>
#include "vk_mem_alloc_wrapper.h"

namespace grom {

class VulkanTexture : public Texture
{
public:
    VulkanTexture() = default;
    ~VulkanTexture() override;

    void* GetHandle() override;
    TextureDesc& GetDesc() override;
    void Upload(void* data, u32 mipLevel) override;

    VkImage GetImage() const { return m_Image; }
    VkImageView GetView() const { return m_View; }
    VmaAllocation GetAllocation() const { return m_Allocation; }
    VkImageView GetDSV() const { return m_DSView; }
    VkImageView GetUAV() const { return m_UAVView; }

    static VulkanTexture* Create(TextureDesc& desc, VkDevice device, VmaAllocator allocator);

private:
    VkImage m_Image = VK_NULL_HANDLE;
    VkImageView m_View = VK_NULL_HANDLE;
    VkImageView m_DSView = VK_NULL_HANDLE;
    VkImageView m_UAVView = VK_NULL_HANDLE;
    VmaAllocation m_Allocation = VK_NULL_HANDLE;
    TextureDesc m_Desc;
    VmaAllocator m_Allocator = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
};

} // namespace grom
