#pragma once

#include <vulkan/vulkan.h>
#include <map>
#include <string>

#include "vulkan_device.h"


struct VulkanPipelineLayout
{

public:
    VkPipelineLayout pipelineLayout;
    std::map<std::string, VkDescriptorSetLayout> descSetLayouts;

    VulkanPipelineLayout(VulkanDevice* vulkanDevice)
        :vulkanDevice(vulkanDevice) {}
    ~VulkanPipelineLayout()
    {
        vkDestroyPipelineLayout(vulkanDevice->vkDevice, pipelineLayout, nullptr);
    }

    VulkanPipelineLayout(const VulkanPipelineLayout&) = delete;
    VulkanPipelineLayout& operator=(const VulkanPipelineLayout&) = delete;

private:
    VulkanDevice* vulkanDevice = nullptr;
};

class PipelineLayoutBuilder
{

public:
    int PushDescriptorSetLayout(
        std::string name,
        std::vector<VkDescriptorSetLayoutBinding> bindings);
    VulkanPipelineLayout BuildPipelineLayout();
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
        VkDescriptorType type, VkShaderStageFlags stageFlags,
        uint32_t binding, uint32_t descriptorCount = 1);

    PipelineLayoutBuilder(VulkanDevice* vulkanDevice);
    ~PipelineLayoutBuilder() = default;

private:
    VulkanDevice *vulkanDevice = nullptr;
    std::map<std::string, VkDescriptorSetLayout> descSetLayouts;
};
