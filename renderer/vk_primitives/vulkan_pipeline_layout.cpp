#include "vulkan_pipeline_layout.h"

#include "vulkan_device.h"
#include "validation.h"

#include <tracy/Tracy.hpp>


void VulkanPipelineLayout::AllocateDescriptorSet(
    std::string name, uint32_t nFrames, VkDescriptorSet* descSet)
{
    ZoneScopedN("VulkanPipelineLayout::AllocateDescriptorSet");

    VkDescriptorSetAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorPool = descriptorPool;
    info.descriptorSetCount = nFrames;
    info.pSetLayouts = &descSetLayouts[name];
    CHECK_VKCMD(vkAllocateDescriptorSets(vulkanDevice->vkDevice, &info, descSet));
}

PipelineLayoutBuilder::PipelineLayoutBuilder(VulkanDevice* vulkanDevice)
{
    ZoneScopedN("PipelineLayoutBuilder::PipelineLayoutBuilder");

    this->vulkanDevice = vulkanDevice;
}

VkDescriptorSetLayoutBinding PipelineLayoutBuilder::descriptorSetLayoutBinding(
    VkDescriptorType type,
    VkShaderStageFlags stageFlags,
    uint32_t binding,
    uint32_t descriptorCount)
{
    ZoneScopedN("PipelineLayoutBuilder::descriptorSetLayoutBinding");

    VkDescriptorSetLayoutBinding setLayoutBinding {};
    setLayoutBinding.descriptorType = type;
    setLayoutBinding.stageFlags = stageFlags;
    setLayoutBinding.binding = binding;
    setLayoutBinding.descriptorCount = descriptorCount;
    return setLayoutBinding;
}

int PipelineLayoutBuilder::PushDescriptorSetLayout(
    std::string name, std::vector<VkDescriptorSetLayoutBinding> bindings)
{
    ZoneScopedN("PipelineLayoutBuilder::PushDescriptorSetLayout");

    VkDescriptorSetLayout descSetLayout;

    VkDescriptorSetLayoutCreateInfo descLayoutInfo{};
    descLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descLayoutInfo.pBindings = bindings.data();
    descLayoutInfo.bindingCount = bindings.size();

    CHECK_VKCMD(vkCreateDescriptorSetLayout(vulkanDevice->vkDevice, 
        &descLayoutInfo, nullptr, &descSetLayout));

    descSetLayouts.push_back({name, descSetLayout});
    return descSetLayouts.size();
}

std::unique_ptr<VulkanPipelineLayout> PipelineLayoutBuilder::BuildPipelineLayout(
    VkDescriptorPool descriptorPool, VkPushConstantRange* pushConst 
){
    ZoneScopedN("PipelineLayoutBuilder::BuildPipelineLayout");

    std::unique_ptr<VulkanPipelineLayout> pipelineLayout
        = std::make_unique<VulkanPipelineLayout>(vulkanDevice, descriptorPool);
    std::vector<VkDescriptorSetLayout> descLayoutList;

    for(auto p = descSetLayouts.begin(); p != descSetLayouts.end(); p++)
    {
        descLayoutList.push_back(p->second);
    }

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = descLayoutList.size();
    layoutInfo.pSetLayouts = descLayoutList.data();

    if (pushConst)
    {
        layoutInfo.pPushConstantRanges = pushConst;
        layoutInfo.pushConstantRangeCount = 1;
    }
    
    for(std::pair e: this->descSetLayouts)
        pipelineLayout->descSetLayouts[e.first] = e.second;
        
    CHECK_VKCMD(vkCreatePipelineLayout(
        vulkanDevice->vkDevice, &layoutInfo,
        nullptr, &pipelineLayout->layout));
    
    return pipelineLayout;
}