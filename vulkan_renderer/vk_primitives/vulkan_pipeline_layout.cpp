#include "vulkan_pipeline_layout.h"

#include "vulkan_device.h"
#include "validation.h"

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

//FIXME: return valid resource
VulkanPipelineLayout PipelineLayoutBuilder::BuildPipelineLayout()
{
    VulkanPipelineLayout pipelineLayout(vulkanDevice);
    std::vector<VkDescriptorSetLayout> descLayoutList;

    for(auto p = descSetLayouts.begin(); p != descSetLayouts.end(); p++)
    {
        descLayoutList.push_back(p->second);
    }

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = descLayoutList.size();
    layoutInfo.pSetLayouts = descLayoutList.data();
    
    CHECK_VKCMD(vkCreatePipelineLayout(
        vulkanDevice->vkDevice, &layoutInfo,
        nullptr, &pipelineLayout.pipelineLayout));
    
    return pipelineLayout;
}