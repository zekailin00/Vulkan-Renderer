#pragma once

#include "camera.h"

#include "vulkan_texture.h"
#include "vk_primitives/vulkan_uniform.h"
#include "vk_primitives/vulkan_device.h"
#include "vulkan_swapchain.h"

#include <vulkan/vulkan.h>
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
    /**
     * Return a VulkanCamera.
     * Note that even though it is a shared pointer.
     * It must only be added to one node.
     * There is no check for it, but can cause undefined error
     * if a camera is added to multiple nodes.
    */
    static std::shared_ptr<VulkanCamera> BuildCamera(CameraProperties&);

    const CameraProperties& GetCamProperties() override;

    void SetCamProperties(CameraProperties&) override;

    const glm::mat4& GetTransform(); // Used by VulkanNode

    void SetTransform(glm::mat4&); // Used by VulkanNode

    void Destroy();

    VulkanCamera() = default;
    ~VulkanCamera() override;

    VulkanCamera(const VulkanCamera&) = delete;
    VulkanCamera& operator=(const VulkanCamera&) = delete;

    // ViewProjection* MapCameraUniform();
    // void BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout layout);
    // VkFramebuffer GetFrameBuffer(){return framebuffer;}
    // VulkanTexture& GetColorImage() {return colorImage;}
    // VkDescriptorSet cameraTexture; /*FIXME: render to texture; used by ImGui. */

private: 
    VulkanTexture colorImage;
    VulkanUniform cameraUniform;
    ViewProjection* vpMap = nullptr;

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