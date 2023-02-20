#pragma once

#include "vulkan_node.h"
#include <vulkan/vulkan.h>

namespace renderer
{

class RenderTechnique
{
public:
    RenderTechnique() = default;
    ~RenderTechnique() = default;

    RenderTechnique(const RenderTechnique&) = delete;
    RenderTechnique& operator=(const RenderTechnique&) = delete;

    void ProcessScene(VulkanNode* root);
    void ExecuteCommand(VkCommandBuffer commandBuffer);
};

} // namespace renderer