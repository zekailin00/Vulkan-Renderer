#include "vulkan_camera.h"

#include "vulkan_renderer.h"
#include "validation.h"

#include <array>

void VulkanCamera::Initialize(glm::vec2 extent, VkFormat vkFormat)
{
    VulkanRenderer& vkr = VulkanRenderer::GetInstance();
    VulkanDevice& vulkanDevice = vkr.vulkanDevice;
    this->extent = extent;

    // Create color image
    colorImage.CreateImage(
        {static_cast<uint32_t>(extent.x), static_cast<uint32_t>(extent.y)},
        vkFormat,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    colorImage.CreateSampler();

    cameraUniform.Initialize(sizeof(ViewProjection));

    // Create depth image
    {
        VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(extent.x);
        imageInfo.extent.height = static_cast<uint32_t>(extent.y);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = vulkanDevice.GetDepthFormat();
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        CHECK_VKCMD(vkCreateImage(vulkanDevice.vkDevice, &imageInfo, nullptr, &depthImage));

        VkMemoryRequirements memRequirements{};
        vkGetImageMemoryRequirements(vulkanDevice.vkDevice, depthImage, &memRequirements);
        VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = vulkanDevice.GetMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        CHECK_VKCMD(vkAllocateMemory(vulkanDevice.vkDevice, &allocInfo, nullptr, &depthMemory));
        CHECK_VKCMD(vkBindImageMemory(vulkanDevice.vkDevice, depthImage, depthMemory, 0));

        VkImageViewCreateInfo depthViewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        depthViewInfo.image = depthImage;
        depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depthViewInfo.format = vulkanDevice.GetDepthFormat();;
        depthViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthViewInfo.subresourceRange.baseMipLevel = 0;
        depthViewInfo.subresourceRange.levelCount = 1;
        depthViewInfo.subresourceRange.baseArrayLayer = 0;
        depthViewInfo.subresourceRange.layerCount = 1;

        CHECK_VKCMD(vkCreateImageView(vulkanDevice.vkDevice, &depthViewInfo, nullptr, &depthImageView));

        VkImageViewCreateInfo stencilViewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        stencilViewInfo.image = depthImage;
        stencilViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        stencilViewInfo.format = vulkanDevice.GetDepthFormat();;
        stencilViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        stencilViewInfo.subresourceRange.baseMipLevel = 0;
        stencilViewInfo.subresourceRange.levelCount = 1;
        stencilViewInfo.subresourceRange.baseArrayLayer = 0;
        stencilViewInfo.subresourceRange.layerCount = 1;

        CHECK_VKCMD(vkCreateImageView(vulkanDevice.vkDevice, &stencilViewInfo, nullptr, &stencilImageView));
    }

    std::vector<VkImageView> attachments = {colorImage.vkImageView, depthImageView};
    VkFramebufferCreateInfo vkFramebufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    vkFramebufferCreateInfo.renderPass = vkr.vkRenderPass.defaultCamera;
    vkFramebufferCreateInfo.attachmentCount = attachments.size();
    vkFramebufferCreateInfo.pAttachments = attachments.data();
    vkFramebufferCreateInfo.width = static_cast<uint32_t>(extent.x);
    vkFramebufferCreateInfo.height = static_cast<uint32_t>(extent.y);
    vkFramebufferCreateInfo.layers = 1;

    CHECK_VKCMD(vkCreateFramebuffer(
        vulkanDevice.vkDevice, 
        &vkFramebufferCreateInfo, nullptr,
        &framebuffer));


    // Create camera descriptor set
    vkr.AllocateDescriptorSet(&cameraDescSet, vkr.vkDescriptorSetLayout.cameraDescLayout);
    std::array<VkWriteDescriptorSet, 1> descriptorWrite{};

    VkDescriptorBufferInfo propBufferInfo = cameraUniform.GetDescriptor();
    descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[0].dstSet = cameraDescSet;
    descriptorWrite[0].dstBinding = 0;
    descriptorWrite[0].dstArrayElement = 0;
    descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite[0].descriptorCount = 1;
    descriptorWrite[0].pBufferInfo = &propBufferInfo;

    vkUpdateDescriptorSets(vkr.vulkanDevice.vkDevice, descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
}

ViewProjection* VulkanCamera::MapCameraUniform()
{
    return static_cast<ViewProjection*>(cameraUniform.Map());
}

void VulkanCamera::BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout layout)
{
    vkCmdBindDescriptorSets(
        commandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        layout, 2, 1, 
        &cameraDescSet, 0, nullptr
    );
}

void VulkanCamera::Destroy()
{
    colorImage.Destroy();
    cameraUniform.Destroy();

    VulkanDevice& vulkanDevice = VulkanRenderer::GetInstance().vulkanDevice;
    vkDestroyImageView(vulkanDevice.vkDevice, depthImageView, nullptr);
    vkDestroyImageView(vulkanDevice.vkDevice, stencilImageView, nullptr);

    vkDestroyImage(vulkanDevice.vkDevice, depthImage, nullptr);
    vkFreeMemory(vulkanDevice.vkDevice, depthMemory, nullptr);

    vkDestroyFramebuffer(vulkanDevice.vkDevice,framebuffer, nullptr);
}