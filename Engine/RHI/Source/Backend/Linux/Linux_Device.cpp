#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#include "RHI/Backend/Linux/Linux_Device.h"
#include "RHI/Backend/Linux/Linux_Texture.h"
#include "RHI/Backend/Linux/Linux_Shader.h"
#include "RHI/Backend/Linux/Linux_Buffer.h"
#include "RHI/Backend/Linux/Linux_Pipeline.h"
#include <cstring>
#include <algorithm>
#include <vector>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <wayland-client.h>

namespace grom {

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
{
    OutputDebugStringA(data->pMessage);
    OutputDebugStringA("\n");
    return VK_FALSE;
}

LinuxDevice::~LinuxDevice()
{
    if (m_Device)
    {
        vkDeviceWaitIdle(m_Device);
        CleanupSwapChain();

        if (m_ImageAvailableSemaphore) vkDestroySemaphore(m_Device, m_ImageAvailableSemaphore, nullptr);
        if (m_RenderFinishedSemaphore) vkDestroySemaphore(m_Device, m_RenderFinishedSemaphore, nullptr);
        if (m_InFlightFence) vkDestroyFence(m_Device, m_InFlightFence, nullptr);
        if (m_CommandPool) vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
        if (m_RenderPass) vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
        
        if (m_Allocator) {
            vmaDestroyAllocator(m_Allocator);
            m_Allocator = VK_NULL_HANDLE;
        }
        
        vkDestroyDevice(m_Device, nullptr);
    }
    if (m_Surface) vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    if (m_Instance) vkDestroyInstance(m_Instance, nullptr);
}

ERenderAPI LinuxDevice::GetAPI() { return ERenderAPI::Vulkan; }
void* LinuxDevice::GetNativeDevice() { return m_Device; }
void* LinuxDevice::GetNativeContext() { return nullptr; }
DeviceDesc& LinuxDevice::GetDesc() { return m_Desc; }

LinuxDevice* LinuxDevice::Create(DeviceDesc& desc)
{
    LinuxDevice* device = new LinuxDevice();
    device->m_Desc = desc;

    if (!device->CreateInstance()) { delete device; return nullptr; }
    if (!device->CreateSurface()) { delete device; return nullptr; }
    if (!device->PickPhysicalDevice()) { delete device; return nullptr; }
    if (!device->CreateLogicalDevice()) { delete device; return nullptr; }
    if (!device->CreateSwapChain()) { delete device; return nullptr; }
    if (!device->CreateRenderPass()) { delete device; return nullptr; }
    device->CreateFramebuffers();
    device->CreateCommandPool();
    device->CreateSyncObjects();

    return device;
}

bool LinuxDevice::CreateInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "GromEngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "GromEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
    extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#ifdef _DEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    std::vector<const char*> layers;
#ifdef _DEBUG
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = static_cast<u32>(layers.size());
    createInfo.ppEnabledLayerNames = layers.data();

    if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
        return false;

#ifdef _DEBUG
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
    if (func)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugInfo.pfnUserCallback = DebugCallback;
        VkDebugUtilsMessengerEXT messenger;
        func(m_Instance, &debugInfo, nullptr, &messenger);
    }
#endif

    return true;
}

bool LinuxDevice::CreateSurface()
{
    // Try Wayland first, fallback to X11
    const char* wl_display = std::getenv("WAYLAND_DISPLAY");
    if (wl_display)
    {
        struct wl_display* display = wl_display_connect(nullptr);
        if (display)
        {
            VkWaylandSurfaceCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
            createInfo.display = display;
            return vkCreateWaylandSurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface) == VK_SUCCESS;
        }
    }

    // Fallback to X11
    Display* x11Display = XOpenDisplay(nullptr);
    if (x11Display)
    {
        Window window = static_cast<Window>(m_Desc.WindowHandle);
        VkXlibSurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        createInfo.dpy = x11Display;
        createInfo.window = window;
        return vkCreateXlibSurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface) == VK_SUCCESS;
    }

    return false;
}

bool LinuxDevice::PickPhysicalDevice()
{
    u32 count = 0;
    vkEnumeratePhysicalDevices(m_Instance, &count, nullptr);
    if (count == 0) return false;

    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(m_Instance, &count, devices.data());

    for (auto& dev : devices)
    {
        u32 qCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &qCount, nullptr);
        std::vector<VkQueueFamilyProperties> queues(qCount);
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &qCount, queues.data());

        for (u32 i = 0; i < qCount; ++i)
        {
            if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                m_GraphicsQueueFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, m_Surface, &presentSupport);
            if (presentSupport)
                m_PresentQueueFamily = i;

            if (m_GraphicsQueueFamily != UINT32_MAX && m_PresentQueueFamily != UINT32_MAX)
                break;
        }

        if (m_GraphicsQueueFamily != UINT32_MAX && m_PresentQueueFamily != UINT32_MAX)
        {
            m_PhysicalDevice = dev;
            break;
        }
    }

    return m_PhysicalDevice != VK_NULL_HANDLE;
}

bool LinuxDevice::CreateLogicalDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    float priority = 1.0f;

    auto addQueue = [&](u32 family)
    {
        VkDeviceQueueCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = family;
        info.queueCount = 1;
        info.pQueuePriorities = &priority;
        queueInfos.push_back(info);
    };

    addQueue(m_GraphicsQueueFamily);
    if (m_GraphicsQueueFamily != m_PresentQueueFamily)
        addQueue(m_PresentQueueFamily);

    std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkPhysicalDeviceFeatures features{};
    features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<u32>(queueInfos.size());
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.pEnabledFeatures = &features;

    if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
        return false;

    vkGetDeviceQueue(m_Device, m_GraphicsQueueFamily, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, m_PresentQueueFamily, 0, &m_PresentQueue);

    // Create VMA allocator
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = m_PhysicalDevice;
    allocatorInfo.device = m_Device;
    allocatorInfo.instance = m_Instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    if (vmaCreateAllocator(&allocatorInfo, &m_Allocator) != VK_SUCCESS)
        return false;

    return true;
}

bool LinuxDevice::CreateSwapChain()
{
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &caps);

    u32 formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, formats.data());

    m_SwapChainFormat = formats[0].format;
    for (auto& fmt : formats)
    {
        if (fmt.format == VK_FORMAT_B8G8R8A8_UNORM && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            m_SwapChainFormat = fmt.format;
            break;
        }
    }

    m_SwapChainExtent = caps.currentExtent;
    if (m_SwapChainExtent.width == UINT32_MAX)
    {
        m_SwapChainExtent.width = std::clamp(m_Desc.Width, caps.minImageExtent.width, caps.maxImageExtent.width);
        m_SwapChainExtent.height = std::clamp(m_Desc.Height, caps.minImageExtent.height, caps.maxImageExtent.height);
    }

    u32 imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
        imageCount = caps.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = m_SwapChainFormat;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = m_SwapChainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    u32 indices[] = { m_GraphicsQueueFamily, m_PresentQueueFamily };
    if (m_GraphicsQueueFamily != m_PresentQueueFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = caps.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
        return false;

    vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
    m_SwapChainImages.Resize(imageCount);
    vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.Data());

    m_SwapChainImageViews.Resize(imageCount);
    for (u32 i = 0; i < imageCount; ++i)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_SwapChainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_SwapChainFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        vkCreateImageView(m_Device, &viewInfo, nullptr, &m_SwapChainImageViews.Data()[i]);
    }

    return true;
}

bool LinuxDevice::CreateRenderPass()
{
    VkAttachmentDescription colorAtt{};
    colorAtt.format = m_SwapChainFormat;
    colorAtt.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAtt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAtt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAtt.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;

    VkSubpassDependency dep{};
    dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.srcAccessMask = 0;
    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &colorAtt;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dep;

    return vkCreateRenderPass(m_Device, &info, nullptr, &m_RenderPass) == VK_SUCCESS;
}

void LinuxDevice::CreateFramebuffers()
{
    m_SwapChainFramebuffers.Resize(m_SwapChainImageViews.Size());
    for (u32 i = 0; i < m_SwapChainImageViews.Size(); ++i)
    {
        VkFramebufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = m_RenderPass;
        info.attachmentCount = 1;
        info.pAttachments = &m_SwapChainImageViews[i];
        info.width = m_SwapChainExtent.width;
        info.height = m_SwapChainExtent.height;
        info.layers = 1;
        vkCreateFramebuffer(m_Device, &info, nullptr, &m_SwapChainFramebuffers.Data()[i]);
    }
}

void LinuxDevice::CreateCommandPool()
{
    VkCommandPoolCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.queueFamilyIndex = m_GraphicsQueueFamily;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_Device, &info, nullptr, &m_CommandPool);

    m_CommandBuffers.Resize(m_SwapChainFramebuffers.Size());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<u32>(m_CommandBuffers.Size());
    vkAllocateCommandBuffers(m_Device, &allocInfo, m_CommandBuffers.Data());
}

void LinuxDevice::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_Device, &semInfo, nullptr, &m_ImageAvailableSemaphore);
    vkCreateSemaphore(m_Device, &semInfo, nullptr, &m_RenderFinishedSemaphore);

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFence);
}

void LinuxDevice::CleanupSwapChain()
{
    for (auto fb : m_SwapChainFramebuffers)
        vkDestroyFramebuffer(m_Device, fb, nullptr);
    for (auto view : m_SwapChainImageViews)
        vkDestroyImageView(m_Device, view, nullptr);
    if (m_SwapChain) vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
}

void LinuxDevice::Present()
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];

    VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(m_Device, 1, &m_InFlightFence);
    vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFence);
    vkWaitForFences(m_Device, 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_SwapChain;
    presentInfo.pImageIndices = &m_CurrentFrame;

    vkQueuePresentKHR(m_PresentQueue, &presentInfo);

    m_CurrentFrame = (m_CurrentFrame + 1) % static_cast<u32>(m_SwapChainImages.Size());
}

void LinuxDevice::Resize(u32 w, u32 h)
{
    m_Desc.Width = w;
    m_Desc.Height = h;
}

void LinuxDevice::ClearRenderTarget(Texture* rt, const f32 color[4]) {}
void LinuxDevice::ClearDepthStencil(Texture* ds, f32 depth, u8 stencil) {}
void LinuxDevice::SetViewport(ViewportDesc& vp) {}
void LinuxDevice::SetScissorRect(Rect2D& rect) {}
void LinuxDevice::SetPipeline(Pipeline* pipeline) {}
void LinuxDevice::SetVertexBuffer(Buffer* buffer, u32 slot) {}
void LinuxDevice::SetIndexBuffer(Buffer* buffer) {}
void LinuxDevice::SetConstantBuffer(Buffer* buffer, u32 slot, EShaderType shader) {}
void LinuxDevice::SetRenderTargets(Texture* const* renderTargets, u32 count, Texture* depthStencil) {}
void LinuxDevice::SetShaderResource(Texture* texture, u32 slot, EShaderType shader) {}
void LinuxDevice::SetUnorderedAccessView(Texture* texture, u32 slot, EShaderType shader) {}
void LinuxDevice::SetBlendState(bool enable, EBlendFactor srcFactor, EBlendFactor dstFactor) {}
void LinuxDevice::Draw(u32 vertexCount, u32 startVertex) {}
void LinuxDevice::DrawIndexed(u32 indexCount, u32 startIndex, u32 baseVertex) {}
void LinuxDevice::Dispatch(u32 groupX, u32 groupY, u32 groupZ) {}

void LinuxDevice::BeginFrame()
{
    vkWaitForFences(m_Device, 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);

    VkCommandBuffer cmd = m_CommandBuffers[m_CurrentFrame];
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(cmd, &beginInfo);

    VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
    VkRenderPassBeginInfo renderInfo{};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderInfo.renderPass = m_RenderPass;
    renderInfo.framebuffer = m_SwapChainFramebuffers[m_CurrentFrame];
    renderInfo.renderArea.extent = m_SwapChainExtent;
    renderInfo.clearValueCount = 1;
    renderInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(cmd, &renderInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void LinuxDevice::EndFrame()
{
    VkCommandBuffer cmd = m_CommandBuffers[m_CurrentFrame];
    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);
}

Texture* LinuxDevice::GetBackBuffer()
{
    return m_BackBufferTexture;
}

} // namespace grom