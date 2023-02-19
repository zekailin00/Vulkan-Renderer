#pragma once

#include "camera.h"

#include "vulkan_texture.h"
#include "vk_primitives/vulkan_uniform.h"
#include "vk_primitives/vulkan_device.h"
#include "vulkan_swapchain.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <memory>

namespace renderer
{

struct ViewProjection
{
    glm::mat4 view;
    glm::mat4 projection;
};

class VulkanCamera: public Camera
{
public:
    static std::unique_ptr<VulkanCamera> BuildCamera(CameraProperties&);

    void Initialize(glm::vec2 extent, VkFormat vkFormat);
    void Destroy();
    ViewProjection* MapCameraUniform();
    void BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout layout);
    VkFramebuffer GetFrameBuffer(){return framebuffer;}
    VulkanTexture& GetColorImage() {return colorImage;}

    VkDescriptorSet cameraTexture; /*FIXME: render to texture; used by ImGui. */
private: 
    VulkanTexture colorImage;
    VulkanUniform cameraUniform;

    VkDescriptorSet cameraDescSet;

    VkImage depthImage{VK_NULL_HANDLE};
    VkDeviceMemory depthMemory{VK_NULL_HANDLE};
    VkImageView depthImageView{VK_NULL_HANDLE};
    VkImageView stencilImageView{VK_NULL_HANDLE};

    VkFramebuffer framebuffer{VK_NULL_HANDLE};

    CameraProperties properties{};
    VulkanDevice* vulkanDevice = nullptr;
    IVulkanSwapchain* swapchain = nullptr;
};


} // namespace renderer