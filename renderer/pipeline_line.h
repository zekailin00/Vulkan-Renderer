#pragma once

#include "vk_primitives/vulkan_device.h"

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

    void Render(std::vector<std::shared_ptr<LineRenderer>> lineList,
        VkBuffer vertexBuffer, VkBuffer indexBuffer, VkCommandBuffer commandbuffer);
};

} // namespace renderer