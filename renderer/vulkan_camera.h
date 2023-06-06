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

class RenderTechnique;

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

    VkFramebuffer GetFrameBuffer(){return framebuffer;}
    VkDescriptorSet* GetDescriptorSet(){return &cameraDescSet;}

    friend RenderTechnique; // Have access to colorTexDescSet

private: 
    VulkanTexture colorImage;
    VulkanUniform cameraUniform;
    ViewProjection* vpMap = nullptr;

    VkDescriptorSet cameraDescSet; // Camera vp descriptor set
    VkDescriptorSet colorTexDescSet; // Rendered texture descriptor set

    VkImage depthImage{VK_NULL_HANDLE};
    VkDeviceMemory depthMemory{VK_NULL_HANDLE};
    VkImageView depthImageView{VK_NULL_HANDLE};
    VkImageView stencilImageView{VK_NULL_HANDLE};

    VkFramebuffer framebuffer{VK_NULL_HANDLE};

    CameraProperties properties{};
    VulkanDevice* vulkanDevice = nullptr;
    IVulkanSwapchain* swapchain = nullptr;
};


class VulkanVrDisplay: public VrDisplay
{

public:
    /**
     * @brief It is only a placeholder so that 
     * there is a VR HMD in the scene.
     * The vulkan resources are initialized when
     * a new XR session is created.
     * 
     * @return std::shared_ptr<VulkanVrDisplay> 
     */
    static std::shared_ptr<VulkanVrDisplay> BuildCamera();

    /**
     * @brief Allocating vulkan resources when a new XR sessions is created.
     * The extent is the resolution of the displays acquired from the session.
     * 
     * @param extent 
     */
    void Initialize(glm::vec2 extent);

    /**
     * @brief When XR session is destroyed,
     * the VR display also needs to be destroyed.
     * 
     */
    void Destory();

    friend RenderTechnique;

private:
    std::shared_ptr<VulkanCamera> cameras[2] = {nullptr, nullptr};
};

} // namespace renderer