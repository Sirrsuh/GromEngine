#pragma once
#include "../../RHI_Device.h"
#include "../../RHI_Texture.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define VK_USE_PLATFORM_WIN32_KHR
#include <Windows.h>
#include <vulkan/vulkan.h>
#include "vk_mem_alloc_wrapper.h"

namespace grom {

class VulkanDevice : public Device
{
public:
    VulkanDevice() = default;
    ~VulkanDevice() override;

    ERenderAPI GetAPI() override;
    void* GetNativeDevice() override;
    void* GetNativeContext() override;
    DeviceDesc& GetDesc() override;
    void Present() override;
    void Resize(u32 w, u32 h) override;
    void ClearRenderTarget(Texture* rt, const f32 color[4]) override;
    void ClearDepthStencil(Texture* ds, f32 depth, u8 stencil) override;
    void SetViewport(ViewportDesc& vp) override;
    void SetScissorRect(Rect2D& rect) override;
    void SetPipeline(Pipeline* pipeline) override;
    void SetVertexBuffer(Buffer* buffer, u32 slot) override;
    void SetIndexBuffer(Buffer* buffer) override;
    void SetConstantBuffer(Buffer* buffer, u32 slot, EShaderType shader) override;
    void SetRenderTargets(Texture* const* renderTargets, u32 count, Texture* depthStencil) override;
    void SetShaderResource(Texture* texture, u32 slot, EShaderType shader) override;
    void SetUnorderedAccessView(Texture* texture, u32 slot, EShaderType shader) override;
    void SetBlendState(bool enable, EBlendFactor srcFactor, EBlendFactor dstFactor) override;
    void Draw(u32 vertexCount, u32 startVertex) override;
    void DrawIndexed(u32 indexCount, u32 startIndex, u32 baseVertex) override;
    void Dispatch(u32 groupX, u32 groupY, u32 groupZ) override;
    void BeginFrame() override;
    void EndFrame() override;
    Texture* GetBackBuffer() override;

    VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
    VkDevice GetVkDevice() const { return m_Device; }
    VkRenderPass GetRenderPass() const { return m_RenderPass; }
    VmaAllocator GetAllocator() const { return m_Allocator; }

    static VulkanDevice* Create(DeviceDesc& desc);

private:
    bool CreateInstance();
    bool CreateSurface();
    bool PickPhysicalDevice();
    bool CreateLogicalDevice();
    bool CreateSwapChain();
    bool CreateRenderPass();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateSyncObjects();
    void CleanupSwapChain();

    VkInstance m_Instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkQueue m_PresentQueue = VK_NULL_HANDLE;
    VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    VkCommandPool m_CommandPool = VK_NULL_HANDLE;

    TArray<VkImage> m_SwapChainImages;
    TArray<VkImageView> m_SwapChainImageViews;
    TArray<VkFramebuffer> m_SwapChainFramebuffers;
    TArray<VkCommandBuffer> m_CommandBuffers;

    VkSemaphore m_ImageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_RenderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence m_InFlightFence = VK_NULL_HANDLE;

    VmaAllocator m_Allocator = VK_NULL_HANDLE;

    Texture* m_BackBufferTexture = nullptr;
    DeviceDesc m_Desc;

    u32 m_GraphicsQueueFamily = UINT32_MAX;
    u32 m_PresentQueueFamily = UINT32_MAX;
    u32 m_CurrentFrame = 0;
    VkFormat m_SwapChainFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkExtent2D m_SwapChainExtent = {};
};

} // namespace grom
