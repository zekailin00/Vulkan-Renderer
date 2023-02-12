#pragma once 

#include "vulkan/vulkan.h"

#include "vk_primitives/vulkan_device.h"
#include "vk_primitives/vulkan_cmdbuffer.h"
#include "vk_primitives/vulkan_pipeline.h"
#include "vulkan_swapchain.h"
#include "vulkan_texture.h"
#include "imgui_plugin.h"
#include "pipeline_inputs.h"
#include "vulkan_camera.h"
#include "vulkan_light.h"

#include <functional>
#include <string>
#include <vector>
#include <map>


//typedef void (*RenderCommand)(VkCommandBuffer);
typedef std::function<void(VkCommandBuffer)> RenderCommand;

struct EditorViewport;

class VulkanRenderer 
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
    VulkanCmdBuffer vulkanCmdBuffer;

#pragma region PipelineContext
    struct {
        VkRenderPass display;
        VkRenderPass defaultCamera;
    } vkRenderPass;

    std::map<std::string, std::unique_ptr<VulkanPipeline>> pipelines;
#pragma endregion PipelineContext 

public:/* Services */
    void RecordCommand(RenderCommand);
    void CleanupCommands();
    VulkanPipelineLayout& GetPipelineLayout(std::string name);
    void AddCamera(VulkanCamera* vulkanCamera, glm::vec2 extent);
    void RemoveCamera(VulkanCamera* vulkanCamera);
    void AddLight(VulkanLight* vulkanLight);
    //TODO: support light removal or multiple lights

private: /* Private Vulkan helpers */
    VulkanRenderer() = default;
    VulkanRenderer(VulkanRenderer const&) = delete;
    void operator=(VulkanRenderer const&) = delete;

    void CreateRenderPasses();
    void CreateFramebuffers();
    void CreatePipelines();

    void DestroyRenderPasses();
    void DestroyFramebuffers();

    void ExecuteRecordedCommands(VkCommandBuffer vkCommandBuffer);
    void DrawCamera(VkCommandBuffer vkCommandBuffer);

public:
    // Descriptor Sets do not support multiple frames in flight.
    // It can only be 1 for now.
    // FIXME: needed to allocate descriptor sets
    // make it public for now
    const uint32_t FRAME_IN_FLIGHT = 1;

private:
    VkDescriptorPool vkDescriptorPool;

    std::vector<VkFramebuffer> vkFramebuffers;

    IVulkanSwapchain* swapchain = nullptr;
    ImguiPlugin imguiPlugin;

    std::vector<RenderCommand> commandQueue;

    std::vector<VulkanCamera*> cameraList{};
    VulkanLight* mainLight;


    bool isEditorEnabled = false;
};