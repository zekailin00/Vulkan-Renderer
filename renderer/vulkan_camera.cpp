#include "vulkan_camera.h"

#include "vulkan_renderer.h"
#include "validation.h"

#include <array>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include<tracy/Tracy.hpp>

namespace renderer
{

std::shared_ptr<VulkanCamera> VulkanCamera::BuildCamera(CameraProperties& properties)
{
    ZoneScopedN("VulkanCamera::BuildCamera");

    std::shared_ptr<VulkanCamera> camera = std::make_unique<VulkanCamera>();

    VulkanRenderer& vkr = VulkanRenderer::GetInstance();
    camera->cameraType = CameraType::CAMERA;
    camera->vulkanDevice = &vkr.vulkanDevice;
    camera->swapchain = vkr.GetSwapchain();

    camera->properties = properties;
    if (camera->properties.UseFrameExtent)
    {
        camera->properties.Extent.x = camera->swapchain->GetWidth();
        camera->properties.Extent.y = camera->swapchain->GetHeight();
    }

    // Create color image
    camera->colorImage.CreateImage({
            static_cast<unsigned int>(camera->properties.Extent.x),
            static_cast<unsigned int>(camera->properties.Extent.y)},
        camera->swapchain->GetImageFormat(),
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    camera->colorImage.CreateSampler();

    camera->cameraUniform.Initialize(camera->vulkanDevice, sizeof(ViewProjection));
    camera->vpMap = static_cast<ViewProjection*>(camera->cameraUniform.Map());
    camera->vpMap->projection = glm::perspective(
        glm::radians(camera->properties.Fov),
        static_cast<float>(camera->properties.Extent.x)
            /static_cast<float>(camera->properties.Extent.y),
        camera->properties.ZNear, camera->properties.ZFar);
    camera->vpMap->view = glm::mat4(1.0f);

    // Create depth image
    {
        VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = camera->properties.Extent.x;
        imageInfo.extent.height = camera->properties.Extent.y;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = camera->vulkanDevice->GetDepthFormat();
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        CHECK_VKCMD(vkCreateImage(
            camera->vulkanDevice->vkDevice, &imageInfo, nullptr, &camera->depthImage));

        VkMemoryRequirements memRequirements{};
        vkGetImageMemoryRequirements(
            camera->vulkanDevice->vkDevice, camera->depthImage, &memRequirements);
        VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = camera->vulkanDevice->GetMemoryTypeIndex(
            memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        CHECK_VKCMD(vkAllocateMemory(
            camera->vulkanDevice->vkDevice, &allocInfo, nullptr, &camera->depthMemory));
        CHECK_VKCMD(vkBindImageMemory(
            camera->vulkanDevice->vkDevice, camera->depthImage, camera->depthMemory, 0));

        VkImageViewCreateInfo depthViewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        depthViewInfo.image = camera->depthImage;
        depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depthViewInfo.format = camera->vulkanDevice->GetDepthFormat();;
        depthViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthViewInfo.subresourceRange.baseMipLevel = 0;
        depthViewInfo.subresourceRange.levelCount = 1;
        depthViewInfo.subresourceRange.baseArrayLayer = 0;
        depthViewInfo.subresourceRange.layerCount = 1;

        CHECK_VKCMD(vkCreateImageView(
            camera->vulkanDevice->vkDevice, &depthViewInfo,
            nullptr, &camera->depthImageView));

        VkImageViewCreateInfo stencilViewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        stencilViewInfo.image = camera->depthImage;
        stencilViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        stencilViewInfo.format = camera->vulkanDevice->GetDepthFormat();
        stencilViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        stencilViewInfo.subresourceRange.baseMipLevel = 0;
        stencilViewInfo.subresourceRange.levelCount = 1;
        stencilViewInfo.subresourceRange.baseArrayLayer = 0;
        stencilViewInfo.subresourceRange.layerCount = 1;

        CHECK_VKCMD(vkCreateImageView(
            camera->vulkanDevice->vkDevice, &stencilViewInfo,
            nullptr, &camera->stencilImageView));
    }

    std::vector<VkImageView> attachments =
        {camera->colorImage.GetImageView(), camera->depthImageView};
    VkFramebufferCreateInfo vkFramebufferCreateInfo{
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    vkFramebufferCreateInfo.renderPass = vkr.vkRenderPass.defaultCamera;
    vkFramebufferCreateInfo.attachmentCount = attachments.size();
    vkFramebufferCreateInfo.pAttachments = attachments.data();
    vkFramebufferCreateInfo.width = camera->properties.Extent.x;
    vkFramebufferCreateInfo.height = camera->properties.Extent.y;
    vkFramebufferCreateInfo.layers = 1;

    CHECK_VKCMD(vkCreateFramebuffer(
        camera->vulkanDevice->vkDevice, 
        &vkFramebufferCreateInfo, nullptr,
        &camera->framebuffer));


    // Create camera descriptor set
    {
        VulkanPipelineLayout& pipelineLayout = vkr.GetPipelineLayout("render");
        pipelineLayout.AllocateDescriptorSet(
            "camera", vkr.FRAME_IN_FLIGHT, &camera->cameraDescSet);
        
        std::array<VkWriteDescriptorSet, 1> descriptorWrite{};

        descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[0].dstSet = camera->cameraDescSet;
        descriptorWrite[0].dstBinding = 0;
        descriptorWrite[0].dstArrayElement = 0;
        descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite[0].descriptorCount = 1;
        descriptorWrite[0].pBufferInfo = camera->cameraUniform.GetDescriptor();

        vkUpdateDescriptorSets(camera->vulkanDevice->vkDevice,
            descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
    }

    // Create rendered texture descriptor set
    {
        VulkanPipelineLayout& pipelineLayout = vkr.GetPipelineLayout("display");
        pipelineLayout.AllocateDescriptorSet(
            "texture", vkr.FRAME_IN_FLIGHT, &camera->colorTexDescSet);
        
        std::array<VkWriteDescriptorSet, 1> descriptorWrite{};

        descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[0].dstSet = camera->colorTexDescSet;
        descriptorWrite[0].dstBinding = 0;
        descriptorWrite[0].dstArrayElement = 0;
        descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite[0].descriptorCount = 1;
        descriptorWrite[0].pImageInfo = camera->colorImage.GetDescriptor();

        vkUpdateDescriptorSets(camera->vulkanDevice->vkDevice,
            descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
    }

    return camera;
}

const CameraProperties& VulkanCamera::GetCamProperties()
{
    ZoneScopedN("VulkanCamera::GetCamProperties");

    return this->properties;
}

void VulkanCamera::SetCamProperties(CameraProperties& properties)
{
    ZoneScopedN("VulkanCamera::SetCamProperties");

    this->properties = properties;
    vpMap->projection = glm::perspective(
        glm::radians(this->properties.Fov),
        static_cast<float>(this->properties.Extent.x)
            /static_cast<float>(this->properties.Extent.y),
        this->properties.ZNear, this->properties.ZFar);
}

const glm::mat4& VulkanCamera::GetTransform()
{
    ZoneScopedN("VulkanCamera::GetTransform");

    return this->vpMap->view;
}

void VulkanCamera::SetTransform(glm::mat4& transform)
{
    ZoneScopedN("VulkanCamera::SetTransform");

    this->vpMap->view = glm::inverse(transform);
}

VulkanCamera::~VulkanCamera()
{
    ZoneScopedN("VulkanCamera::~VulkanCamera");

    Destroy();
}

void VulkanCamera::Destroy()
{
    ZoneScopedN("VulkanCamera::Destroy");

    vkDeviceWaitIdle(vulkanDevice->vkDevice);

    colorImage.Destroy();
    cameraUniform.Destroy();

    vkDestroyImageView(vulkanDevice->vkDevice, depthImageView, nullptr);
    vkDestroyImageView(vulkanDevice->vkDevice, stencilImageView, nullptr);

    vkDestroyImage(vulkanDevice->vkDevice, depthImage, nullptr);
    vkFreeMemory(vulkanDevice->vkDevice, depthMemory, nullptr);

    vkDestroyFramebuffer(vulkanDevice->vkDevice,framebuffer, nullptr);

    vulkanDevice = nullptr;
    swapchain = nullptr;
    vpMap = nullptr;
}

std::shared_ptr<VulkanVrDisplay> VulkanVrDisplay::BuildCamera()
{
    ZoneScopedN("VulkanVrDisplay::BuildCamera");

    std::shared_ptr<VulkanVrDisplay> display =
        std::make_shared<VulkanVrDisplay>();
    display->cameraType = CameraType::VR_DISPLAY;

    return display;
}

void VulkanVrDisplay::Initialize(glm::vec2 extent)
{
    ZoneScopedN("VulkanVrDisplay::Initialize");

    CameraProperties prop{};
    prop.UseFrameExtent = false;
    prop.Extent = extent;

    cameras[0] = VulkanCamera::BuildCamera(prop);
    cameras[1] = VulkanCamera::BuildCamera(prop);
}

void VulkanVrDisplay::Destory()
{
    ZoneScopedN("VulkanVrDisplay::Destory");

    cameras[0] = nullptr;
    cameras[1] = nullptr;
}

} // namespace renderer