#include "vulkan_pipeline_layout.h"

#include "vulkan_device.h"
#include "validation.h"

void VulkanPipelineLayout::AllocateDescriptorSet(
    std::string name, uint32_t nFrames, VkDescriptorSet* descSet)
{
    VkDescriptorSetAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorPool = descriptorPool;
    info.descriptorSetCount = nFrames;
    info.pSetLayouts = &descSetLayouts[name];
    CHECK_VKCMD(vkAllocateDescriptorSets(vulkanDevice->vkDevice, &info, descSet));
}

PipelineLayoutBuilder::PipelineLayoutBuilder(VulkanDevice* vulkanDevice)
{
    this->vulkanDevice = vulkanDevice;
}

VkDescriptorSetLayoutBinding PipelineLayoutBuilder::descriptorSetLayoutBinding(
    VkDescriptorType type,
    VkShaderStageFlags stageFlags,
    uint32_t binding,
    uint32_t descriptorCount)
{
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
    VkDescriptorSetLayout descSetLayout;

    VkDescriptorSetLayoutCreateInfo descLayoutInfo{};
    descLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descLayoutInfo.pBindings = bindings.data();
    descLayoutInfo.bindingCount = bindings.size();

    CHECK_VKCMD(vkCreateDescriptorSetLayout(vulkanDevice->vkDevice, 
        &descLayoutInfo, nullptr, &descSetLayout));

    descSetLayouts[name] = descSetLayout;
    return descSetLayouts.size();
}

std::unique_ptr<VulkanPipelineLayout> PipelineLayoutBuilder::BuildPipelineLayout(
    VkDescriptorPool descriptorPool
){
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
    
    pipelineLayout->descSetLayouts = this->descSetLayouts;
    CHECK_VKCMD(vkCreatePipelineLayout(
        vulkanDevice->vkDevice, &layoutInfo,
        nullptr, &pipelineLayout->layout));
    
    return pipelineLayout;
}