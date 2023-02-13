#pragma once

#include <vulkan/vulkan.h>
#include <map>
#include <memory>
#include <string>

#include "vulkan_device.h"


class VulkanPipelineLayout
{

public:
    VkPipelineLayout layout;
    std::map<std::string, VkDescriptorSetLayout> descSetLayouts;

    VulkanPipelineLayout(
        VulkanDevice* vulkanDevice, VkDescriptorPool descriptorPool)
        :vulkanDevice(vulkanDevice), descriptorPool(descriptorPool) {}
    ~VulkanPipelineLayout()
    {
        vkDestroyPipelineLayout(vulkanDevice->vkDevice, layout, nullptr);
        for(auto e: descSetLayouts)
        {
            vkDestroyDescriptorSetLayout(vulkanDevice->vkDevice, e.second, nullptr);
        }
    }

    void AllocateDescriptorSet(
        std::string name, uint32_t nFrames, VkDescriptorSet* descSet);

    VulkanPipelineLayout(const VulkanPipelineLayout&) = delete;
    VulkanPipelineLayout& operator=(const VulkanPipelineLayout&) = delete;

private: // owned by VulkanRenderer
    VulkanDevice* vulkanDevice = nullptr;
    VkDescriptorPool descriptorPool;
};

class PipelineLayoutBuilder
{

public:
    int PushDescriptorSetLayout(
        std::string name,
        std::vector<VkDescriptorSetLayoutBinding> bindings);
    std::unique_ptr<VulkanPipelineLayout> BuildPipelineLayout(
        VkDescriptorPool descriptorPool);
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
        VkDescriptorType type, VkShaderStageFlags stageFlags,
        uint32_t binding, uint32_t descriptorCount = 1);

    PipelineLayoutBuilder(VulkanDevice* vulkanDevice);
    ~PipelineLayoutBuilder() = default;

private:
    VulkanDevice *vulkanDevice = nullptr;
    std::vector<std::pair<std::string, VkDescriptorSetLayout>> descSetLayouts;
};
