#include "vulkan_pipeline.h"

#include "vulkan_shader.h"
#include "validation.h"
#include "logger.h"

#include <string>


VulkanPipeline::VulkanPipeline(VkDevice device)
{
    this->device = device;

    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    rasterState.sType = 
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterState.cullMode = VK_CULL_MODE_FRONT_BIT;
    rasterState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterState.depthClampEnable = VK_FALSE;
    rasterState.rasterizerDiscardEnable = VK_FALSE;
    rasterState.depthBiasEnable = VK_FALSE;
    rasterState.lineWidth = 1.0f;

    blendAttachment.blendEnable = 0;
    blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    blendAttachment.colorWriteMask = 
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    colorBlend.sType = 
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlend.attachmentCount = 1;
    colorBlend.pAttachments = &blendAttachment;
    colorBlend.logicOpEnable = VK_FALSE;
    colorBlend.logicOp = VK_LOGIC_OP_NO_OP;
    colorBlend.blendConstants[0] = 1.0f;
    colorBlend.blendConstants[1] = 1.0f;
    colorBlend.blendConstants[2] = 1.0f;
    colorBlend.blendConstants[3] = 1.0f;

    viewPortState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewPortState.viewportCount = 1;
    viewPortState.scissorCount = 1;

    depthStencilState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.stencilTestEnable = VK_FALSE;
    depthStencilState.minDepthBounds = 0.0f;
    depthStencilState.maxDepthBounds = 1.0f;

    multisampleState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    dynamicStateInfo.sType = 
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.pDynamicStates = dynamicStateEnables.data();
    dynamicStateInfo.dynamicStateCount = dynamicStateEnables.size();

    vkPipelineInfo.sType =
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkPipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    vkPipelineInfo.pStages = shaderStages.data();

    vkPipelineInfo.pInputAssemblyState = &inputAssembly;
    vkPipelineInfo.pViewportState = &viewPortState;
    vkPipelineInfo.pRasterizationState = &rasterState;
    vkPipelineInfo.pMultisampleState = &multisampleState;
    vkPipelineInfo.pDepthStencilState = &depthStencilState;
    vkPipelineInfo.pColorBlendState = &colorBlend;
    vkPipelineInfo.pDynamicState = &dynamicStateInfo;

    vkPipelineInfo.subpass = 0;
}

VulkanPipeline::~VulkanPipeline()
{
    vkDestroyPipeline(device, pipeline, nullptr);
}

void VulkanPipeline::LoadShader(std::string vertPath, std::string fragPath)
{
    shaderStages = {
        VulkanShader::LoadFromFile(device, vertPath, VK_SHADER_STAGE_VERTEX_BIT),
        VulkanShader::LoadFromFile(device, fragPath, VK_SHADER_STAGE_FRAGMENT_BIT)
    };
}

void VulkanPipeline::BuildPipeline(
    VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
    VkPipelineLayout pipelineLayout, VkRenderPass renderPass
){
    if (shaderStages.size() != 2)
        Log::Write(Log::Level::Error, 
            "[Vulkan Pipeline] Shaders loadded incorrectly.");

    vkPipelineInfo.pVertexInputState = vertexInputInfo;
    vkPipelineInfo.layout = pipelineLayout;
    vkPipelineInfo.renderPass = renderPass;

    CHECK_VKCMD(vkCreateGraphicsPipelines(
        device, VK_NULL_HANDLE, 1, &vkPipelineInfo, nullptr, &pipeline));  
}
