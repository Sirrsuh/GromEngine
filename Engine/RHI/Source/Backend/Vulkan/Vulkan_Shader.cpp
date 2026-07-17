#define VK_USE_PLATFORM_WIN32_KHR
#include "RHI/Backend/Vulkan/Vulkan_Shader.h"

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
    if (!device || desc.Source.IsEmpty())
        return nullptr;

    // Expect Source to contain SPIR-V binary data
    const uint32_t* code = reinterpret_cast<const uint32_t*>(desc.Source.c_str());
    size_t codeSize = desc.Source.Len();

    if (codeSize % 4 != 0)
        return nullptr; // SPIR-V must be 4-byte aligned

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = codeSize;
    createInfo.pCode = code;

    VkShaderModule module;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &module) != VK_SUCCESS)
        return nullptr;

    VkShaderStageFlagBits stageMap[] = {
        VK_SHADER_STAGE_VERTEX_BIT,   // EShaderType::Vertex = 0
        VK_SHADER_STAGE_FRAGMENT_BIT, // EShaderType::Pixel
        VK_SHADER_STAGE_COMPUTE_BIT,  // EShaderType::Compute
        VK_SHADER_STAGE_GEOMETRY_BIT, // EShaderType::Geometry
        VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, // EShaderType::Hull
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, // EShaderType::Domain
    };

    if (static_cast<int>(desc.Type) >= 6)
    {
        vkDestroyShaderModule(device, module, nullptr);
        return nullptr;
    }

    VulkanShader* shader = new VulkanShader();
    shader->m_Device = device;
    shader->m_Module = module;
    shader->m_Desc = desc;
    shader->m_Stage = stageMap[static_cast<int>(desc.Type)];
    return shader;
}

} // namespace grom