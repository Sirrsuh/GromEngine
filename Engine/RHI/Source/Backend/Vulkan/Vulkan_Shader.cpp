#include "RHI/Backend/Vulkan/Vulkan_Shader.h"
#include <fstream>
#include <vector>

namespace grom {

VulkanShader::~VulkanShader()
{
    if (m_Module)
        vkDestroyShaderModule(m_Device, m_Module, nullptr);
}

void* VulkanShader::GetHandle() { return m_Module; }
ShaderDesc& VulkanShader::GetDesc() { return m_Desc; }

VulkanShader* VulkanShader::Create(ShaderDesc& desc, VkDevice device)
{
    if (!device || desc.DataSize == 0)
        return nullptr;

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = desc.DataSize;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(desc.Data);

    VkShaderModule module;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &module) != VK_SUCCESS)
        return nullptr;

    VkShaderStageFlagBits stageMap[] = {
        VK_SHADER_STAGE_VERTEX_BIT,   // EShaderType::Vertex = 0
        VK_SHADER_STAGE_FRAGMENT_BIT, // EShaderType::Pixel
        VK_SHADER_STAGE_COMPUTE_BIT,  // EShaderType::Compute
        VK_SHADER_STAGE_ALL_GRAPHICS, // EShaderType::Geometry
        VK_SHADER_STAGE_ALL_GRAPHICS, // EShaderType::Hull
        VK_SHADER_STAGE_ALL_GRAPHICS, // EShaderType::Domain
    };

    if (static_cast<int>(desc.Type) >= 6)
        return nullptr;

    VulkanShader* shader = new VulkanShader();
    shader->m_Device = device;
    shader->m_Module = module;
    shader->m_Desc = desc;
    shader->m_Stage = stageMap[static_cast<int>(desc.Type)];
    return shader;
}

} // namespace grom
