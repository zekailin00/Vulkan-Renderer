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

    void Initialize(CameraProperties& prop);
        
    void Destroy();

    void RebuildCamera(CameraProperties& prop);

    const CameraProperties& GetCamProperties() override;

    /* FIXME: has a bug */
    void SetCamProperties(CameraProperties&) override;

    /**
     * @brief Set projection.
     * Usually used for regular cameras
     * 
     * @param aspectRatioXy aspect ratio: x over y
     * @param fovy field of view of y axis in degree
     * @param zNear near plane in meter
     * @param zFar far plane in mater
     */
    void SetProjection(float aspectRatioXy, float fovy = 45.0f,
        float zNear = 0.1f, float zFar = 100.0f);

    /**
     * @brief Set projection.
     * Usually used for fov data from openxr system
     * 
     * @param fov <left, right, up, down> in ????
     * @param zNear near plane in meter
     * @param zFar far plane in meter
     */
    void SetProjection(glm::vec4 fov, float zNear, float zFar);

    const glm::mat4& GetTransform(); // Used by VulkanNode

    void SetTransform(const glm::mat4&); // Used by VulkanNode

    VulkanCamera() = default;
    ~VulkanCamera() override;

    VulkanCamera(const VulkanCamera&) = delete;
    VulkanCamera& operator=(const VulkanCamera&) = delete;

    VkFramebuffer GetFrameBuffer(){return framebuffer;}
    VkDescriptorSet* GetDescriptorSet(){return &cameraDescSet;}
    VkDescriptorSet* GetTextureDescriptorSet(){return &colorTexDescSet;}

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
     * @brief Build two regular cameras.
     * 
     * @return std::shared_ptr<VulkanVrDisplay> 
     */
    static std::shared_ptr<VulkanVrDisplay> BuildCamera();

    /**
     * @brief Rebuild the two cameras when a new XR sessions is created.
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

    ~VulkanVrDisplay();

    std::shared_ptr<VulkanCamera> GetLeftCamera() {return cameras[0];}
    std::shared_ptr<VulkanCamera> GetRightCamera() {return cameras[1];}

private:
    // index 0 = left eye, index 1 = right eye
    std::shared_ptr<VulkanCamera> cameras[2] = {nullptr, nullptr};
};

} // namespace renderer