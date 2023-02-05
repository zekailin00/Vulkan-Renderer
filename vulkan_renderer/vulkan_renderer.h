#pragma once 

#include "vulkan/vulkan.h"

#include "vk_primitives/vulkan_device.h"
#include "vulkan_cmdbuffer.h"
#include "vulkan_swapchain.h"
#include "vulkan_texture.h"
#include "imgui_plugin.h"
#include "pipeline_inputs.h"
#include "vulkan_camera.h"
#include "vulkan_light.h"

#include <vector>
#include <functional>


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
        //VkPipelineLayout display; // Only for VR and in-game quad rendering, not editor
        VkPipelineLayout phongShading;
    } vkPipelineLayout;

    struct {
        VkDescriptorSetLayout materialDescLayout;
        VkDescriptorSetLayout modelDescLayout;
        VkDescriptorSetLayout cameraDescLayout;
        VkDescriptorSetLayout sceneDescLayout;
    } vkDescriptorSetLayout;

    struct {
        VkRenderPass display;
        VkRenderPass defaultCamera;
    } vkRenderPass;

    struct {
        //VkPipeline display; // Only for VR and in-game quad rendering, not editor
        VkPipeline phongShading;
        VkPipeline skyBox;
    } vkPipeline;
#pragma endregion PipelineContext 

public:/* Services */
    void RecordCommand(RenderCommand);
    void CleanupCommands();
    VkPipelineLayout GetPhongPipelineLayout() {return vkPipelineLayout.phongShading;}
    void AllocateDescriptorSet(VkDescriptorSet* descSet, VkDescriptorSetLayout descLayout);
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
    void CreateDescriptorSetLayout();
    void CreatePipelineCache();
    void CreatePipelines();

    void DestroyRenderPasses();
    void DestroyFramebuffers();
    void DestroyDescriptorSetLayout();
    void DestroyPipelineCache();
    void DestroyPipelines();

    void ExecuteRecordedCommands(VkCommandBuffer vkCommandBuffer);
    void DrawCamera(VkCommandBuffer vkCommandBuffer);

private:
    // Descriptor Sets do not support multiple frames in flight.
    // It can only be 1 for now.
    const uint32_t FRAME_IN_FLIGHT = 1;

    VkDescriptorPool vkDescriptorPool;

    std::vector<VkFramebuffer> vkFramebuffers;

    VkPipelineCache vkPipelineCache;

    IVulkanSwapchain* swapchain = nullptr;
    ImguiPlugin imguiPlugin;

    std::vector<RenderCommand> commandQueue;

    std::vector<VulkanCamera*> cameraList{};
    // VulkanCamera* mainCamera;
    VulkanLight* mainLight;
    // VulkanTextureColor2D defaultEditorImage;

    bool isEditorEnabled = false;
};