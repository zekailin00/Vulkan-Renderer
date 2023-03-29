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

#include "loaders/objloader.h"
#include "loaders/gltfloader.h"

#include <vulkan/vulkan.h>
#include <functional>
#include <string>
#include <vector>
#include <map>

namespace renderer
{


//typedef void (*RenderCommand)(VkCommandBuffer);
typedef std::function<void(VkCommandBuffer)> RenderCommand;

class VulkanRenderer: public Renderer
{
public:
    static VulkanRenderer& GetInstance()
    {
        static VulkanRenderer vulkanRenderer;
        return vulkanRenderer;
    }

    /* Update loop APIs */
    void InitializeDevice(uint32_t extensionsCount, const char** extensions);
    void AllocateResources(IVulkanSwapchain* swapchain);
    void BeginFrame();
    void EndFrame();
    void DeallocateResources();
    void Destroy();
    void RebuildSwapchain();

public:
    VkInstance vkInstance;
    VulkanDevice vulkanDevice;

    struct {
        VkRenderPass display;
        VkRenderPass defaultCamera;
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
    void RecordCommand(RenderCommand);
    void CleanupCommands();
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

    void CreateRenderPasses();
    void CreateFramebuffers();
    void CreatePipelines();

    void DestroyRenderPasses();
    void DestroyFramebuffers();

    void ExecuteRecordedCommands(VkCommandBuffer vkCommandBuffer);

public:
    // Descriptor Sets do not support multiple frames in flight.
    // It can only be 1 for now.
    // FIXME: needed to allocate descriptor sets
    // make it public for now
    const uint32_t FRAME_IN_FLIGHT = 1;

private:
    VkDescriptorPool vkDescriptorPool;

    // Framebuffers are created from images returned from the window system
    std::vector<VkFramebuffer> vkFramebuffers;
    
    VulkanCmdBuffer vulkanCmdBuffer;

    std::map<std::string, std::unique_ptr<VulkanPipeline>> pipelines;

    IVulkanSwapchain* swapchain = nullptr;

    std::vector<RenderCommand> commandQueue;

    RenderTechnique defaultTechnique{};
};


} // namespace renderer