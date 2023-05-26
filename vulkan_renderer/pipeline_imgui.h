#pragma once

#include "vulkan_texture.h"
#include "vk_primitives/vulkan_pipeline.h"

#include <imgui.h>
#include <memory>
#include <glm/vec2.hpp>


namespace renderer
{

class Node;

class PipelineImgui
{

public:
    PipelineImgui(
        VulkanDevice& vulkanDevice,
        VkDescriptorPool vkDescriptorPool,
        VkRenderPass renderpass);
    ~PipelineImgui();

    PipelineImgui(const PipelineImgui&) = delete;
    const PipelineImgui& operator=(const PipelineImgui&) = delete;

    void RenderUI(ImDrawData* drawData,
        VkBuffer vertexBuffer, VkBuffer indexBuffer, VkCommandBuffer commandbuffer);

private:
    std::shared_ptr<VulkanTexture> fontTexture;
    std::unique_ptr<VulkanPipeline> imguiPipeline;

    VkDescriptorSet fontTextureDescSet;

    // Cached data
    VulkanDevice vulkanDevice;
    VkDescriptorPool vkDescriptorPool;

    struct ImguiPushConst
    {
        glm::vec2 uScale;
        glm::vec2 uTranslate;
    };
};

} // namespace renderer
