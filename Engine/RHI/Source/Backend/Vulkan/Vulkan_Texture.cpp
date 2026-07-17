#define VK_USE_PLATFORM_WIN32_KHR
#include "RHI/Backend/Vulkan/Vulkan_Texture.h"

namespace grom {

VulkanTexture::~VulkanTexture()
{
    if (m_UAVView) vkDestroyImageView(m_Device, m_UAVView, nullptr);
    if (m_DSView) vkDestroyImageView(m_Device, m_DSView, nullptr);
    if (m_View) vkDestroyImageView(m_Device, m_View, nullptr);
    if (m_Image) vkDestroyImage(m_Device, m_Image, nullptr);
    if (m_Memory) vkFreeMemory(m_Device, m_Memory, nullptr);
}

void* VulkanTexture::GetHandle() { return m_Image; }
TextureDesc& VulkanTexture::GetDesc() { return m_Desc; }
void VulkanTexture::Upload(void*, u32) {}

static VkFormat ConvertFormat(EFormat fmt)
{
    switch (fmt)
    {
    case EFormat::R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
    case EFormat::R8G8B8A8_SRGB:  return VK_FORMAT_R8G8B8A8_SRGB;
    case EFormat::B8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
    case EFormat::R16G16B16A16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
    case EFormat::R32G32B32A32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case EFormat::D32_FLOAT:      return VK_FORMAT_D32_SFLOAT;
    case EFormat::R32_FLOAT:      return VK_FORMAT_R32_SFLOAT;
    case EFormat::R8_UNORM:       return VK_FORMAT_R8_UNORM;
    default: return VK_FORMAT_UNDEFINED;
    }
}

static VkImageUsageFlags ConvertUsage(const TextureDesc& desc)
{
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (desc.IsRenderTarget)       usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (desc.IsDepthStencil)       usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (true)                      usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (desc.IsUnorderedAccess)    usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    return usage;
}

static VkImageAspectFlags GetAspect(const TextureDesc& desc)
{
    if (desc.IsDepthStencil)
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    return VK_IMAGE_ASPECT_COLOR_BIT;
}

VulkanTexture* VulkanTexture::Create(TextureDesc& desc, VkDevice device, VkPhysicalDevice physDevice)
{
    VkFormat vkFormat = ConvertFormat(desc.Format);
    if (vkFormat == VK_FORMAT_UNDEFINED)
        return nullptr;

    VkImageUsageFlags usage = ConvertUsage(desc);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = vkFormat;
    imageInfo.extent = { desc.Width, desc.Height, 1 };
    imageInfo.mipLevels = desc.MipLevels > 0 ? desc.MipLevels : 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = usage;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image;
    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
        return nullptr;

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(device, image, &memReqs);

    VkPhysicalDeviceMemoryProperties physMemProps;
    vkGetPhysicalDeviceMemoryProperties(physDevice, &physMemProps);

    u32 memoryType = UINT32_MAX;
    for (u32 i = 0; i < physMemProps.memoryTypeCount; ++i)
    {
        if ((memReqs.memoryTypeBits & (1 << i)) &&
            (physMemProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        {
            memoryType = i;
            break;
        }
    }

    if (memoryType == UINT32_MAX)
    {
        vkDestroyImage(device, image, nullptr);
        return nullptr;
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = memoryType;

    VkDeviceMemory memory;
    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
    {
        vkDestroyImage(device, image, nullptr);
        return nullptr;
    }

    vkBindImageMemory(device, image, memory, 0);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = vkFormat;
    viewInfo.subresourceRange.aspectMask = GetAspect(desc);
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView view;
    if (vkCreateImageView(device, &viewInfo, nullptr, &view) != VK_SUCCESS)
    {
        vkFreeMemory(device, memory, nullptr);
        vkDestroyImage(device, image, nullptr);
        return nullptr;
    }

    VkImageView dsView = VK_NULL_HANDLE;
    if (desc.IsDepthStencil)
    {
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        vkCreateImageView(device, &viewInfo, nullptr, &dsView);
    }

    VkImageView uavView = VK_NULL_HANDLE;
    if (desc.IsUnorderedAccess)
    {
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vkCreateImageView(device, &viewInfo, nullptr, &uavView);
    }

    VulkanTexture* tex = new VulkanTexture();
    tex->m_Device = device;
    tex->m_Image = image;
    tex->m_View = view;
    tex->m_DSView = dsView;
    tex->m_UAVView = uavView;
    tex->m_Memory = memory;
    tex->m_Desc = desc;
    return tex;
}

} // namespace grom
