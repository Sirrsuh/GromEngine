#pragma once
#include "../../RHI_Texture.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <vulkan/vulkan.h>

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
    VkDeviceMemory GetMemory() const { return m_Memory; }
    VkImageView GetDSV() const { return m_DSView; }
    VkImageView GetUAV() const { return m_UAVView; }

    static VulkanTexture* Create(TextureDesc& desc, VkDevice device, VkPhysicalDevice physDevice);

private:
    VkImage m_Image = VK_NULL_HANDLE;
    VkImageView m_View = VK_NULL_HANDLE;
    VkImageView m_DSView = VK_NULL_HANDLE;
    VkImageView m_UAVView = VK_NULL_HANDLE;
    VkDeviceMemory m_Memory = VK_NULL_HANDLE;
    TextureDesc m_Desc;
    VkDevice m_Device = VK_NULL_HANDLE;
};

} // namespace grom
