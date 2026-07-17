#define VK_USE_PLATFORM_WIN32_KHR
#include "RHI/Backend/Vulkan/Vulkan_Pipeline.h"
#include <vector>

namespace grom {

VulkanPipeline::~VulkanPipeline()
{
    if (m_Pipeline) vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
    if (m_Layout) vkDestroyPipelineLayout(m_Device, m_Layout, nullptr);
}

void* VulkanPipeline::GetHandle() { return m_Pipeline; }
PipelineDesc& VulkanPipeline::GetDesc() { return m_Desc; }

VulkanPipeline* VulkanPipeline::Create(PipelineDesc& desc, VkDevice device, VkRenderPass renderPass)
{
    VulkanPipeline* pipeline = new VulkanPipeline();
    pipeline->m_Device = device;
    pipeline->m_Desc = desc;

    std::vector<VkPipelineShaderStageCreateInfo> stages;

    auto addStage = [&](VkShaderModule module, VkShaderStageFlagBits stage)
    {
        VkPipelineShaderStageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.stage = stage;
        info.module = module;
        info.pName = "main";
        stages.push_back(info);
    };

    if (desc.VS)
    {
        VulkanShader* vs = static_cast<VulkanShader*>(desc.VS);
        addStage(vs->GetModule(), vs->GetStage());
    }
    if (desc.PS)
    {
        VulkanShader* ps = static_cast<VulkanShader*>(desc.PS);
        addStage(ps->GetModule(), ps->GetStage());
    }

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;

    if (desc.InputLayout.Elements.Size() > 0)
    {
        auto& layout = desc.InputLayout;
        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = layout.Stride;
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindings.push_back(binding);

        for (u32 i = 0; i < layout.Elements.Size(); ++i)
        {
            auto& elem = layout.Elements[i];
            VkFormat fmt = VK_FORMAT_R32G32B32A32_SFLOAT;

            auto formatSize = [](EFormat f) -> u32
            {
                switch (f)
                {
                case EFormat::R32G32B32A32_FLOAT: return 16;
                case EFormat::R32G32B32_FLOAT:    return 12;
                case EFormat::R32G32_FLOAT:       return 8;
                case EFormat::R32_FLOAT:          return 4;
                default: return 4;
                }
            };

            switch (elem.Format)
            {
            case EFormat::R32G32B32A32_FLOAT: fmt = VK_FORMAT_R32G32B32A32_SFLOAT; break;
            case EFormat::R32G32B32_FLOAT:    fmt = VK_FORMAT_R32G32B32_SFLOAT; break;
            case EFormat::R32G32_FLOAT:       fmt = VK_FORMAT_R32G32_SFLOAT; break;
            case EFormat::R32_FLOAT:          fmt = VK_FORMAT_R32_SFLOAT; break;
            default: break;
            }

            VkVertexInputAttributeDescription attr{};
            attr.location = i;
            attr.binding = 0;
            attr.format = fmt;
            attr.offset = elem.Offset;
            attributes.push_back(attr);
        }

        vertexInput.vertexBindingDescriptionCount = static_cast<u32>(bindings.size());
        vertexInput.pVertexBindingDescriptions = bindings.data();
        vertexInput.vertexAttributeDescriptionCount = static_cast<u32>(attributes.size());
        vertexInput.pVertexAttributeDescriptions = attributes.data();
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport vp{};
    vp.width = 800.0f;
    vp.height = 600.0f;
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.extent = { 800, 600 };

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &vp;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample{};
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = desc.DepthStencil.DepthEnable ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = desc.DepthStencil.DepthWrite ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

    VkPipelineColorBlendAttachmentState blendAtt{};
    blendAtt.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAtt.blendEnable = desc.Blend.Enable ? VK_TRUE : VK_FALSE;

    if (blendAtt.blendEnable)
    {
        auto convFactor = [](EBlendFactor f) -> VkBlendFactor
        {
            switch (f)
            {
            case EBlendFactor::One:           return VK_BLEND_FACTOR_ONE;
            case EBlendFactor::Zero:          return VK_BLEND_FACTOR_ZERO;
            case EBlendFactor::SrcAlpha:      return VK_BLEND_FACTOR_SRC_ALPHA;
            case EBlendFactor::InvSrcAlpha:   return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            default: return VK_BLEND_FACTOR_ONE;
            }
        };
        blendAtt.srcColorBlendFactor = convFactor(desc.Blend.SrcFactor);
        blendAtt.dstColorBlendFactor = convFactor(desc.Blend.DstFactor);
        blendAtt.colorBlendOp = VK_BLEND_OP_ADD;
        blendAtt.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAtt.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAtt.alphaBlendOp = VK_BLEND_OP_ADD;
    }

    VkPipelineColorBlendStateCreateInfo blendState{};
    blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendState.attachmentCount = 1;
    blendState.pAttachments = &blendAtt;

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipeline->m_Layout) != VK_SUCCESS)
    {
        delete pipeline;
        return nullptr;
    }

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.stageCount = static_cast<u32>(stages.size());
    createInfo.pStages = stages.data();
    createInfo.pVertexInputState = &vertexInput;
    createInfo.pInputAssemblyState = &inputAssembly;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterizer;
    createInfo.pMultisampleState = &multisample;
    createInfo.pDepthStencilState = &depthStencil;
    createInfo.pColorBlendState = &blendState;
    createInfo.layout = pipeline->m_Layout;
    createInfo.renderPass = renderPass;

    if (desc.CS)
    {
        // Compute pipeline
        VulkanShader* cs = static_cast<VulkanShader*>(desc.CS);

        VkComputePipelineCreateInfo compInfo{};
        compInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        compInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        compInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        compInfo.stage.module = cs->GetModule();
        compInfo.stage.pName = "main";
        compInfo.layout = pipeline->m_Layout;

        pipeline->m_BindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

        if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &compInfo, nullptr, &pipeline->m_Pipeline) != VK_SUCCESS)
        {
            delete pipeline;
            return nullptr;
        }

        return pipeline;
    }

    pipeline->m_BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline->m_Pipeline) != VK_SUCCESS)
    {
        delete pipeline;
        return nullptr;
    }

    return pipeline;
}

} // namespace grom
