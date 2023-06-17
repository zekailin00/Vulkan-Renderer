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

    // Create camera descriptor set
    {
        VulkanPipelineLayout& pipelineLayout = vkr.GetPipelineLayout("render");
        pipelineLayout.AllocateDescriptorSet(
            "camera", vkr.FRAME_IN_FLIGHT, &camera->cameraDescSet);
    }

    // Create rendered texture descriptor set
    {
        VulkanPipelineLayout& pipelineLayout = vkr.GetPipelineLayout("display");
        pipelineLayout.AllocateDescriptorSet(
            "texture", vkr.FRAME_IN_FLIGHT, &camera->colorTexDescSet);
    }

    camera->Initialize(properties);

    return camera;
}

void VulkanCamera::Initialize(CameraProperties& prop)
{
    VulkanRenderer& vkr = VulkanRenderer::GetInstance();

    this->properties = prop;
    if (properties.UseFrameExtent)
    {
        properties.Extent.x = swapchain->GetWidth();
        properties.Extent.y = swapchain->GetHeight();
    }

    // Create color image
    this->colorImage.CreateImage({
            static_cast<unsigned int>(this->properties.Extent.x),
            static_cast<unsigned int>(this->properties.Extent.y)},
        this->swapchain->GetImageFormat(),
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    this->colorImage.CreateSampler();

    this->cameraUniform.Initialize(this->vulkanDevice, sizeof(ViewProjection));
    this->vpMap = static_cast<ViewProjection*>(this->cameraUniform.Map());
    this->vpMap->projection = glm::perspective(
        glm::radians(this->properties.Fov),
        static_cast<float>(this->properties.Extent.x)
            /static_cast<float>(this->properties.Extent.y),
        this->properties.ZNear, this->properties.ZFar);
    this->vpMap->view = glm::mat4(1.0f);

    // Create depth image
    {
        VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = this->properties.Extent.x;
        imageInfo.extent.height = this->properties.Extent.y;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = this->vulkanDevice->GetDepthFormat();
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        CHECK_VKCMD(vkCreateImage(
            this->vulkanDevice->vkDevice, &imageInfo, nullptr, &this->depthImage));

        VkMemoryRequirements memRequirements{};
        vkGetImageMemoryRequirements(
            this->vulkanDevice->vkDevice, this->depthImage, &memRequirements);
        VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = this->vulkanDevice->GetMemoryTypeIndex(
            memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        CHECK_VKCMD(vkAllocateMemory(
            this->vulkanDevice->vkDevice, &allocInfo, nullptr, &this->depthMemory));
        CHECK_VKCMD(vkBindImageMemory(
            this->vulkanDevice->vkDevice, this->depthImage, this->depthMemory, 0));

        VkImageViewCreateInfo depthViewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        depthViewInfo.image = this->depthImage;
        depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depthViewInfo.format = this->vulkanDevice->GetDepthFormat();;
        depthViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthViewInfo.subresourceRange.baseMipLevel = 0;
        depthViewInfo.subresourceRange.levelCount = 1;
        depthViewInfo.subresourceRange.baseArrayLayer = 0;
        depthViewInfo.subresourceRange.layerCount = 1;

        CHECK_VKCMD(vkCreateImageView(
            this->vulkanDevice->vkDevice, &depthViewInfo,
            nullptr, &this->depthImageView));

        VkImageViewCreateInfo stencilViewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        stencilViewInfo.image = this->depthImage;
        stencilViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        stencilViewInfo.format = this->vulkanDevice->GetDepthFormat();
        stencilViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        stencilViewInfo.subresourceRange.baseMipLevel = 0;
        stencilViewInfo.subresourceRange.levelCount = 1;
        stencilViewInfo.subresourceRange.baseArrayLayer = 0;
        stencilViewInfo.subresourceRange.layerCount = 1;

        CHECK_VKCMD(vkCreateImageView(
            this->vulkanDevice->vkDevice, &stencilViewInfo,
            nullptr, &this->stencilImageView));
    }

    std::vector<VkImageView> attachments =
        {this->colorImage.GetImageView(), this->depthImageView};
    VkFramebufferCreateInfo vkFramebufferCreateInfo{
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    vkFramebufferCreateInfo.renderPass = vkr.vkRenderPass.defaultCamera;
    vkFramebufferCreateInfo.attachmentCount = attachments.size();
    vkFramebufferCreateInfo.pAttachments = attachments.data();
    vkFramebufferCreateInfo.width = this->properties.Extent.x;
    vkFramebufferCreateInfo.height = this->properties.Extent.y;
    vkFramebufferCreateInfo.layers = 1;

    CHECK_VKCMD(vkCreateFramebuffer(
        this->vulkanDevice->vkDevice, 
        &vkFramebufferCreateInfo, nullptr,
        &this->framebuffer));

    {
        std::array<VkWriteDescriptorSet, 1> descriptorWrite{};

        descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[0].dstSet = this->cameraDescSet;
        descriptorWrite[0].dstBinding = 0;
        descriptorWrite[0].dstArrayElement = 0;
        descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite[0].descriptorCount = 1;
        descriptorWrite[0].pBufferInfo = this->cameraUniform.GetDescriptor();

        vkUpdateDescriptorSets(this->vulkanDevice->vkDevice,
            descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
    }

    {
        std::array<VkWriteDescriptorSet, 1> descriptorWrite{};

        descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[0].dstSet = this->colorTexDescSet;
        descriptorWrite[0].dstBinding = 0;
        descriptorWrite[0].dstArrayElement = 0;
        descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite[0].descriptorCount = 1;
        descriptorWrite[0].pImageInfo = this->colorImage.GetDescriptor();

        vkUpdateDescriptorSets(this->vulkanDevice->vkDevice,
            descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
    }
}

void VulkanCamera::RebuildCamera(CameraProperties& prop)
{
    Destroy();
    Initialize(prop);
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

void VulkanCamera::SetProjection(float aspectRatioXy, float fovy,
    float zNear, float zFar)
{
    properties.Fov = fovy;
    properties.ZFar = zFar;
    properties.ZNear = zNear;
    this->vpMap->projection = glm::perspective(
        glm::radians(fovy), aspectRatioXy, zNear, zFar);
}

const glm::mat4& VulkanCamera::GetTransform()
{
    ZoneScopedN("VulkanCamera::GetTransform");

    return this->vpMap->view;
}

void VulkanCamera::SetTransform(const glm::mat4& transform)
{
    ZoneScopedN("VulkanCamera::SetTransform");

    this->vpMap->view = glm::inverse(transform);
}

VulkanCamera::~VulkanCamera()
{
    ZoneScopedN("VulkanCamera::~VulkanCamera");

    Destroy();
    vulkanDevice = nullptr;
    swapchain = nullptr;
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