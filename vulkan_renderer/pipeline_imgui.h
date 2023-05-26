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

    void RenderUI(ImDrawData* drawData, VkCommandBuffer commandbuffer);

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

    struct FrameRenderBuffers
    {
        VkDeviceMemory      VertexBufferMemory = VK_NULL_HANDLE;
        VkDeviceMemory      IndexBufferMemory = VK_NULL_HANDLE;
        VkDeviceSize        VertexBufferSize = 0;
        VkDeviceSize        IndexBufferSize = 0;
        VkBuffer            VertexBuffer = VK_NULL_HANDLE;
        VkBuffer            IndexBuffer = VK_NULL_HANDLE;
    };

    // TODO: It does not support double buffering.
    FrameRenderBuffers renderBuffers;

private: //helpers for rendering
    void CreateOrResizeBuffer(
        VkBuffer& buffer, VkDeviceMemory& buffer_memory,
        VkDeviceSize& p_buffer_size, size_t new_size, VkBufferUsageFlagBits usage);

};

} // namespace renderer
