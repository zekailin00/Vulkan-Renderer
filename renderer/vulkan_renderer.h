#pragma once 

#include "renderer.h"

#include "vk_primitives/vulkan_device.h"
#include "vk_primitives/vulkan_cmdbuffer.h"
#include "vk_primitives/vulkan_pipeline.h"

#include "renderer_assets.h"

#include "vulkan_swapchain.h"
#include "render_technique.h"
#include "pipeline_imgui.h"

#include <vulkan/vulkan.h>
#include <string>
#include <map>
#include <memory>

namespace renderer
{

class VulkanRenderer: public Renderer
{
public:
    static VulkanRenderer& GetInstance();

    /* Update loop APIs */
    void InitializeDevice(
        std::vector<const char *> instanceExt, std::vector<const char *> deviceExt);
    void AllocateResources(
        IVulkanSwapchain* glfwSwapchain, AssetManager* assetManager);
    void BeginFrame();
    void EndFrame();
    void DeallocateResources();
    void Destroy();
    void RebuildSwapchain();

    bool InitializeXrSession(IVulkanSwapchain* xrSwapchain);
    void DestroyXrSession();

    AssetManager* GetAssetManager() {return assetManager;}

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

    AssetManager* assetManager = nullptr;

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