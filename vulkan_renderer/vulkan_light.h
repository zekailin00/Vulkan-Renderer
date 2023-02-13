#pragma once

#include "vk_primitives/vulkan_uniform.h"
#include "pipeline_inputs.h"

#include <vulkan/vulkan.h>

class VulkanLight
{
public:
    void Initialize();
    void Destroy();
    LightProperties* MapCameraUniform();
    void BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout layout);

private: 
    VulkanUniform lightUniform;
    VkDescriptorSet lightDescSet;
};