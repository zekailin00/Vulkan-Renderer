#pragma once 

#include "renderer.h"

#include "vk_primitives/vulkan_device.h"
#include "vk_primitives/vulkan_cmdbuffer.h"
#include "vk_primitives/vulkan_pipeline.h"

#include "vulkan_swapchain.h"
#include "vulkan_texture.h"
#include "pipeline_inputs.h"
#include "vulkan_camera.h"
#include "vulkan_light.h"
#include "render_technique.h"
#include "pipeline_imgui.h"

#include "loaders/objloader.h"
#include "loaders/gltfloader.h"

#include <vulkan/vulkan.h>
#include <functional>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <tracy/Tracy.hpp>

namespace renderer
{

class VulkanRenderer: public Renderer
{
public:
    static VulkanRenderer& GetInstance()
    {
        ZoneScopedN("VulkanRenderer::GetInstance");
        static VulkanRenderer vulkanRenderer;
        return vulkanRenderer;
    }

    /* Update loop APIs */
    void InitializeDevice(
        std::vector<const char *> instanceExt, std::vector<const char *> deviceExt);
    void AllocateResources(
        IVulkanSwapchain* glfwSwapchain, IVulkanSwapchain* openxrSwapchain);
    void BeginFrame();
    void EndFrame();
    void DeallocateResources();
    void Destroy();
    void RebuildSwapchain();

    bool InitializeXrSession(IVulkanSwapchain* xrSwapchain);
    void DestroyXrSession();

public:
    VkInstance vkInstance;
    VulkanDevice vulkanDevice;

    friend RenderTechnique;

    struct {
        VkRenderPass display;
        VkRenderPass defaultCamera;
        VkRenderPass imgui;
    } vkRenderPass;
    /**
     * FIXME:
     * How to refactor renderpass
     * * a single renderpass can be used for multiple framebuffers and multiple pipelines
     * so to handle it:
     * create an abstract base class as render context (framebuffer)
     * each child of abstract base class has to implement a renderpass
     * so that each instance(framebuffer) share one same renderpass, 
     * and can pass the renderpass to pipeline builder.
    */

public:/* Services */
    VulkanPipelineLayout& GetPipelineLayout(std::string name);
    VulkanPipeline& GetPipeline(std::string name);
    IVulkanSwapchain* GetSwapchain() {return swapchain;}

    Scene* CreateScene() override;
    Scene* GetScene() override;    

private: /* Private Vulkan helpers */
    VulkanRenderer() = default;
    ~VulkanRenderer() = default;
    VulkanRenderer(VulkanRenderer const&) = delete;
    void operator=(VulkanRenderer const&) = delete;

    void RenderOpenxrFrame(VkCommandBuffer vkCommandBuffer);

    void CreateRenderPasses();
    void CreateFramebuffers();
    void CreatePipelines();

    void DestroyRenderPasses();
    void DestroyFramebuffers();

public:
    // Descriptor Sets do not support multiple frames in flight.
    // It can only be 1 for now.
    // FIXME: needed to allocate descriptor sets
    // make it public for now
    const uint32_t FRAME_IN_FLIGHT = 1;

private:
    VkDescriptorPool vkDescriptorPool;
    VulkanCmdBuffer vulkanCmdBuffer;

    std::map<std::string, std::unique_ptr<VulkanPipeline>> pipelines;
    std::unique_ptr<PipelineImgui> pipelineImgui;

    // Owned by glfw
    IVulkanSwapchain* swapchain = nullptr;

    struct OpenxrContext
    {
        IVulkanSwapchain* swapchain;
        VkRenderPass renderpass;
        VulkanVrDisplay* display;
        std::unique_ptr<VulkanPipeline> pipeline;
    };

    OpenxrContext* xrContext = nullptr;

    RenderTechnique defaultTechnique{};
};


} // namespace renderer