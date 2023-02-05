#pragma once


#include <vulkan/vulkan.h>
#include <vector>

class VulkanPipeline
{
public:
    VulkanPipeline(VkDevice device);

    ~VulkanPipeline();

    void LoadShader(std::string vertPath, std::string fragPath);

    void BuildPipeline(VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
        VkPipelineLayout pipelineLayout, VkRenderPass renderPass);

    VulkanPipeline(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;
    
public:
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    VkPipelineRasterizationStateCreateInfo rasterState{};
    VkPipelineColorBlendAttachmentState blendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlend{};
    VkPipelineViewportStateCreateInfo viewPortState{};
    VkPipelineDepthStencilStateCreateInfo depthStencilState{};
    VkPipelineMultisampleStateCreateInfo multisampleState{};
    std::vector<VkDynamicState> dynamicStateEnables{};
    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
    VkGraphicsPipelineCreateInfo vkPipelineInfo{};

public:
    VkPipeline pipeline;
    VkDevice device;
};