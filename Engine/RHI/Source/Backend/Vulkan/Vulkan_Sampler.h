#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <vulkan/vulkan.h>

namespace grom {

class VulkanSampler
{
public:
    VulkanSampler() = default;
    ~VulkanSampler() { if (m_Sampler) vkDestroySampler(m_Device, m_Sampler, nullptr); }

    VkSampler GetSampler() const { return m_Sampler; }

    bool Create(VkDevice device, VkFilter minFilter, VkFilter magFilter,
                VkSamplerAddressMode addressMode, bool anisotropy = false)
    {
        m_Device = device;

        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = magFilter;
        info.minFilter = minFilter;
        info.addressModeU = addressMode;
        info.addressModeV = addressMode;
        info.addressModeW = addressMode;
        info.anisotropyEnable = anisotropy ? VK_TRUE : VK_FALSE;
        info.maxAnisotropy = anisotropy ? 16.0f : 1.0f;
        info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        info.unnormalizedCoordinates = VK_FALSE;
        info.compareEnable = VK_FALSE;
        info.compareOp = VK_COMPARE_OP_ALWAYS;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        return vkCreateSampler(device, &info, nullptr, &m_Sampler) == VK_SUCCESS;
    }

private:
    VkSampler m_Sampler = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
};

} // namespace grom
