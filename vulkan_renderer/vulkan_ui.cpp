#include "vulkan_ui.h"

#include "vulkan_renderer.h"
#include "validation.h"

#include <memory>

namespace renderer
{

std::shared_ptr<VulkanUI> VulkanUI::BuildUI(UIBuildInfo& info)
{
    std::shared_ptr<VulkanUI> ui = std::make_shared<VulkanUI>();
    ui->Initialize(info.extent, info.imgui);

    return ui;
}

VulkanUI::~VulkanUI()
{
    Destroy();
}

void VulkanUI::Destroy()
{
    vkDeviceWaitIdle(vulkanDevice->vkDevice);

    this->colorImage = nullptr;
    vkDestroyFramebuffer(vulkanDevice->vkDevice,framebuffer, nullptr);

    this->vulkanDevice = nullptr;
    this->renderUI = nullptr;
}

void VulkanUI::RenderUI()
{
    ImGui::NewFrame();
    ImGui::SetNextWindowPos({0.0f, 0.0f});
    ImGui::SetNextWindowSize({extent.x, extent.y});
    ImGui::Begin("wwww");
    this->renderUI();
    ImGui::End();
    ImGui::EndFrame();
    ImGui::Render();
    this->drawData = ImGui::GetDrawData();
}

void VulkanUI::SetExtent(glm::vec2 extent)
{
    void (*renderUI)(void) = this->renderUI;
    Destroy();
    Initialize(extent, renderUI);
}

std::shared_ptr<Texture> VulkanUI::GetTexture()
{
    return colorImage;
}

void VulkanUI::Initialize(glm::vec2& extent, void (*renderUI)(void))
{
    this->extent = extent;
    this->renderUI = renderUI;

    VulkanRenderer& vkr = VulkanRenderer::GetInstance();
    IVulkanSwapchain *swapchain = vkr.GetSwapchain();
    this->vulkanDevice = &vkr.vulkanDevice;

    // Create color image
    this->colorImage = std::make_shared<VulkanTexture>();
    this->colorImage->CreateImage({
            static_cast<unsigned int>(this->extent.x),
            static_cast<unsigned int>(this->extent.y)},
        swapchain->GetImageFormat(),
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    this->colorImage->CreateSampler();

    std::vector<VkImageView> attachments = {this->colorImage->GetImageView()};
    VkFramebufferCreateInfo vkFramebufferCreateInfo{
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    vkFramebufferCreateInfo.renderPass = vkr.vkRenderPass.imgui;
    vkFramebufferCreateInfo.attachmentCount = attachments.size();
    vkFramebufferCreateInfo.pAttachments = attachments.data();
    vkFramebufferCreateInfo.width = this->extent.x;
    vkFramebufferCreateInfo.height = this->extent.y;
    vkFramebufferCreateInfo.layers = 1;

    CHECK_VKCMD(vkCreateFramebuffer(
        this->vulkanDevice->vkDevice, 
        &vkFramebufferCreateInfo, nullptr,
        &this->framebuffer));
}

} // namespace renderer