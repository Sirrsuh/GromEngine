#pragma once
#include "../../RHI_Pipeline.h"
#include "Vulkan_Shader.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <vulkan/vulkan.h>

namespace grom {

class VulkanPipeline : public Pipeline
{
public:
    VulkanPipeline() = default;
    ~VulkanPipeline() override;

    void* GetHandle() override;
    PipelineDesc& GetDesc() override;

    VkPipeline GetPipeline() const { return m_Pipeline; }
    VkPipelineLayout GetLayout() const { return m_Layout; }
    VkPipelineBindPoint GetBindPoint() const { return m_BindPoint; }

    static VulkanPipeline* Create(PipelineDesc& desc, VkDevice device, VkRenderPass renderPass);

private:
    VkPipeline m_Pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_Layout = VK_NULL_HANDLE;
    VkPipelineBindPoint m_BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    PipelineDesc m_Desc;
    VkDevice m_Device = VK_NULL_HANDLE;
};

} // namespace grom
