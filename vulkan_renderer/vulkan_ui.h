#pragma once

#include "ui.h"

#include "vk_primitives/vulkan_device.h"
#include "vulkan_texture.h"

#include <glm/vec2.hpp>
#include <memory>

namespace renderer
{

class VulkanUI: public UI
{

public:
    static std::shared_ptr<VulkanUI> BuildUI(UIBuildInfo& info);

    VulkanUI() = default;
    ~VulkanUI() override;

    VulkanUI(const VulkanUI&) = delete;
    VulkanUI& operator=(const VulkanUI&) = delete;

    void SetExtent(glm::vec2 extent) override;

    void Destroy();

    void RenderUI();

private:
    glm::vec2 extent;
    void (*renderUI)(void);

    VulkanDevice* vulkanDevice;
    VulkanTexture colorImage;
    VkFramebuffer framebuffer;

private:
    void Initialize(glm::vec2& extent, void (*renderUI)(void));
};

} // namespace renderer
