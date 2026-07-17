#pragma once
#include "../../RHI_Shader.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <vulkan/vulkan.h>

namespace grom {

class VulkanShader : public Shader
{
public:
    VulkanShader() = default;
    ~VulkanShader() override;

    void* GetHandle() override;
    ShaderDesc& GetDesc() override;

    VkShaderModule GetModule() const { return m_Module; }
    VkShaderStageFlagBits GetStage() const { return m_Stage; }

    static VulkanShader* Create(ShaderDesc& desc, VkDevice device);

private:
    VkShaderModule m_Module = VK_NULL_HANDLE;
    VkShaderStageFlagBits m_Stage = {};
    ShaderDesc m_Desc;
    VkDevice m_Device = VK_NULL_HANDLE;
};

} // namespace grom
