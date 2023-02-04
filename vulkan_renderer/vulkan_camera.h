#pragma once 

#include "vulkan_texture.h"
#include "vulkan_uniform.h"
#include "pipeline_inputs.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

class VulkanCamera
{
public:
    glm::vec2 extent;

    void Initialize(glm::vec2 extent, VkFormat vkFormat);
    void Destroy();
    ViewProjection* MapCameraUniform();
    void BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout layout);
    VkFramebuffer GetFrameBuffer(){return framebuffer;}
    VulkanTextureColor2D& GetColorImage() {return colorImage;}

    VkDescriptorSet cameraTexture; /*FIXME: render to texture; used by ImGui. */
private: 
    VulkanTextureColor2D colorImage;
    VulkanUniform cameraUniform;

    VkDescriptorSet cameraDescSet;

    VkImage depthImage{VK_NULL_HANDLE};
    VkDeviceMemory depthMemory{VK_NULL_HANDLE};
    VkImageView depthImageView{VK_NULL_HANDLE};
    VkImageView stencilImageView{VK_NULL_HANDLE};

    VkFramebuffer framebuffer{VK_NULL_HANDLE};
};