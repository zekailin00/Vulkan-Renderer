#pragma once

#include "vk_primitives/vulkan_device.h"
#include "vk_primitives/vulkan_pipeline.h"

#include "vulkan_wireframe.h"

#include <vector>
#include <memory>


namespace renderer
{

class PipelineLine
{

public:
    PipelineLine(
        VulkanDevice& vulkanDevice,
        VkDescriptorPool vkDescriptorPool,
        VkRenderPass renderpass);
    ~PipelineLine();

    PipelineLine(const PipelineLine&) = delete;
    void operator=(const PipelineLine&) = delete;

    void Render(std::vector<std::shared_ptr<LineRenderer>>& lineList,
        VkDescriptorSet* cameraDescSet, glm::vec2 extent,
        VkCommandBuffer commandBuffer);

    VulkanPipeline* GetVulkanPipeline()
    {
        return linePipeline.get();
    }
    
public:
    VulkanDevice* vulkanDevice;
    VkDescriptorPool vkDescriptorPool;
    std::unique_ptr<VulkanPipeline> linePipeline;
};

} // namespace renderer