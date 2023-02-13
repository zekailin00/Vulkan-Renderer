#include "vulkan_light.h"

#include "vulkan_renderer.h"

#include <array>

void VulkanLight::Initialize()
{
    VulkanRenderer& vkr = VulkanRenderer::GetInstance();
    VulkanDevice& vulkanDevice = vkr.vulkanDevice;

    lightUniform.Initialize(&vulkanDevice, sizeof(LightProperties));

    // Create light descriptor set
    VulkanPipelineLayout& pipelineLayout = vkr.GetPipelineLayout("render");
    pipelineLayout.AllocateDescriptorSet("scene", vkr.FRAME_IN_FLIGHT, &lightDescSet);
    
    std::array<VkWriteDescriptorSet, 1> descriptorWrite{};

    VkDescriptorBufferInfo propBufferInfo = lightUniform.GetDescriptor();
    descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[0].dstSet = lightDescSet;
    descriptorWrite[0].dstBinding = 0;
    descriptorWrite[0].dstArrayElement = 0;
    descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite[0].descriptorCount = 1;
    descriptorWrite[0].pBufferInfo = &propBufferInfo;

    vkUpdateDescriptorSets(vkr.vulkanDevice.vkDevice, descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
}

LightProperties* VulkanLight::MapCameraUniform()
{
    return static_cast<LightProperties*>(lightUniform.Map());
}

void VulkanLight::BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout layout)
{
    vkCmdBindDescriptorSets(
        commandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        layout, 3, 1, 
        &lightDescSet, 0, nullptr
    );
}

void VulkanLight::Destroy()
{
    lightUniform.Destroy();
}