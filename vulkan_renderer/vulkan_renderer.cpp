#include "vulkan_renderer.h"

#include "vulkan_swapchain.h"
#include "pipeline_inputs.h"
#include "vulkan_utilities.h"
#include "vulkan_texture.h"

#include "vk_primitives/vulkan_vertexbuffer.h"
#include "vk_primitives/vulkan_device.h"
#include "vk_primitives/vulkan_cmdbuffer.h"
#include "vk_primitives/vulkan_shader.h"
#include "vk_primitives/vulkan_pipeline.h"

#include "validation.h"
#include "logger.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <iostream>
#include <vector>
#include <array>
#include <stddef.h> // offset(type, member)

#define VULKAN_DEBUG_REPORT
#define VULKAN_APPLE_SUPPORT

#ifdef VULKAN_DEBUG_REPORT
static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    (void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
    fprintf(stderr, "[Vulkan Renderer] Debug report from ObjectType: %i at location %x.\n\tMessage: %s\n\n", objectType, location, pMessage);
    return VK_FALSE;
}
#endif

void VulkanRenderer::InitializeDevice(uint32_t extensionsCount, const char** extensions)
{
    VkResult err;

    // Create Vulkan Instance
    {
        std::vector<const char*> extensionList;
        extensionList.resize(extensionsCount);
        memcpy(extensionList.data(), extensions, extensionsCount * sizeof(const char*));

        VkInstanceCreateInfo vkCreateInfo = {};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

#ifdef VULKAN_APPLE_SUPPORT
        vkCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        extensionList.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        extensionList.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

#ifdef VULKAN_DEBUG_REPORT
        // Enabling validation layers
        const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
        vkCreateInfo.enabledLayerCount = 1;
        vkCreateInfo.ppEnabledLayerNames = layers;

        // Enable debug report extension
        extensionList.push_back("VK_EXT_debug_report");
        vkCreateInfo.enabledExtensionCount = extensionList.size();
        vkCreateInfo.ppEnabledExtensionNames = extensionList.data();

        // Create Vulkan Instance
        err = vkCreateInstance(&vkCreateInfo, nullptr, &vkInstance);
        CHECK_VKCMD(err);

        // Get the function pointer (required for any extensions)
        auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");
        ASSERT(vkCreateDebugReportCallbackEXT != NULL);

        // Setup the debug report callback
        VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
        debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debug_report_ci.pfnCallback = debug_report;
        debug_report_ci.pUserData = NULL;
        err = vkCreateDebugReportCallbackEXT(vkInstance, &debug_report_ci, nullptr, &g_DebugReport);
        CHECK_VKCMD(err);
#else
        // Create Vulkan Instance without any debug feature
        vkCreateInfo.enabledExtensionCount = extensionList.size();
        vkCreateInfo.ppEnabledExtensionNames = extensionList.data();
        CHECK_VKCMD(vkCreateInstance(&vkCreateInfo, nullptr, &vkInstance));
#endif
    }

    vulkanDevice.Initialize(vkInstance);
}

void VulkanRenderer::AllocateResources(IVulkanSwapchain* swapchain)
{
    this->swapchain = swapchain;
    swapchain->Initialize();

    std::vector<VkDescriptorPoolSize> poolSizes =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo vkDescriptorPoolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    vkDescriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    vkDescriptorPoolInfo.maxSets = 1000 * static_cast<uint32_t>(poolSizes.size());
    vkDescriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    vkDescriptorPoolInfo.pPoolSizes = poolSizes.data();
    CHECK_VKCMD(vkCreateDescriptorPool(vulkanDevice.vkDevice, &vkDescriptorPoolInfo, nullptr, &vkDescriptorPool));

    vulkanCmdBuffer.Initialize(&vulkanDevice, FRAME_IN_FLIGHT);
    
    CreateRenderPasses();
    CreatePipelines();
    CreateFramebuffers();

    // Initialize ImGui Plugin
    ImGui_ImplVulkan_InitInfo imguiVulkanInitInfo{};
    imguiVulkanInitInfo.Allocator = nullptr;
    imguiVulkanInitInfo.CheckVkResultFn = nullptr;
    imguiVulkanInitInfo.DescriptorPool = vkDescriptorPool;
    imguiVulkanInitInfo.Device = vulkanDevice.vkDevice;
    imguiVulkanInitInfo.ImageCount = swapchain->GetImageCount();
    imguiVulkanInitInfo.Instance = vkInstance;
    imguiVulkanInitInfo.MinImageCount = swapchain->GetImageCount();
    imguiVulkanInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    imguiVulkanInitInfo.PhysicalDevice = vulkanDevice.vkPhysicalDevice;
    imguiVulkanInitInfo.PipelineCache = VkPipelineCache();
    imguiVulkanInitInfo.Queue = vulkanDevice.graphicsQueue;
    imguiVulkanInitInfo.QueueFamily = vulkanDevice.graphicsIndex;
    imguiVulkanInitInfo.Subpass = 0;

    // Use any command buffer. vkDeviceWaitIdle will wait until all are executed.
    imguiPlugin.InitializePlugin(imguiVulkanInitInfo, vkRenderPass.display);

    Log::Write(Log::Level::Info, "Size of RenderCommand: " + std::to_string(sizeof(RenderCommand)));
}

void VulkanRenderer::RebuildSwapchain()
{
    vkDeviceWaitIdle(vulkanDevice.vkDevice);

    DestroyFramebuffers();
    swapchain->RebuildSwapchain();
    CreateFramebuffers();
}

void VulkanRenderer::CreateRenderPasses()
{
    VkAttachmentDescription displayAttachmentDesc{};
    displayAttachmentDesc.format = swapchain->GetImageFormat(); // From swapchain. Hard-coded.
    displayAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    displayAttachmentDesc.loadOp =  VK_ATTACHMENT_LOAD_OP_CLEAR;
    displayAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    displayAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    displayAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    displayAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    displayAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachmentDesc{};
    depthAttachmentDesc.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentDesc{};
    colorAttachmentDesc.format = swapchain->GetImageFormat();
    colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference displayAttachment{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference colorAttachment{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depthAttachment{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    // Display render pass.
    {
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &displayAttachment;
        
        // Dependency for vkQueuePresentKHR with attachment output semaphore.
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo vkRenderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        vkRenderPassCreateInfo.attachmentCount = 1;
        vkRenderPassCreateInfo.pAttachments = &displayAttachmentDesc;
        vkRenderPassCreateInfo.subpassCount = 1;
        vkRenderPassCreateInfo.pSubpasses = &subpass;
        vkRenderPassCreateInfo.dependencyCount = 1;
        vkRenderPassCreateInfo.pDependencies = &dependency;

        CHECK_VKCMD(vkCreateRenderPass(vulkanDevice.vkDevice, &vkRenderPassCreateInfo, nullptr, &vkRenderPass.display));
    }

    // Default camera render pass
    {

        std::vector<VkAttachmentDescription> attachmentDesc{colorAttachmentDesc, depthAttachmentDesc};

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachment;
        subpass.pDepthStencilAttachment = &depthAttachment;

        // Dependency for vkQueuePresentKHR with attachment output semaphore.
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo vkRenderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        vkRenderPassCreateInfo.attachmentCount = 2;
        vkRenderPassCreateInfo.pAttachments = attachmentDesc.data();
        vkRenderPassCreateInfo.subpassCount = 1;
        vkRenderPassCreateInfo.pSubpasses = &subpass;
        vkRenderPassCreateInfo.dependencyCount = 1;
        vkRenderPassCreateInfo.pDependencies = &dependency;

        CHECK_VKCMD(vkCreateRenderPass(vulkanDevice.vkDevice, &vkRenderPassCreateInfo, nullptr, &vkRenderPass.defaultCamera));
    }

}

void VulkanRenderer::DestroyRenderPasses()
{
    vkDestroyRenderPass(vulkanDevice.vkDevice, vkRenderPass.defaultCamera, nullptr);
    vkDestroyRenderPass(vulkanDevice.vkDevice, vkRenderPass.display, nullptr);
}

void VulkanRenderer::CreateFramebuffers()
{
    VkImageView attachment[1];
    VkFramebufferCreateInfo vkFramebufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    vkFramebufferCreateInfo.renderPass = vkRenderPass.display;
    vkFramebufferCreateInfo.attachmentCount = 1;
    vkFramebufferCreateInfo.pAttachments = attachment;
    vkFramebufferCreateInfo.width = swapchain->GetWidth();
    vkFramebufferCreateInfo.height = swapchain->GetHeight();
    vkFramebufferCreateInfo.layers = 1;
    //TODO: wrap framebuffer into swapchain class
    vkFramebuffers.resize(swapchain->GetImageCount());
    for (uint32_t i = 0; i < swapchain->GetImageCount(); i++)
    {
        attachment[0] = swapchain->GetImageView(i);
        CHECK_VKCMD(vkCreateFramebuffer(vulkanDevice.vkDevice, &vkFramebufferCreateInfo, nullptr, &vkFramebuffers[i]));
    }
}

void VulkanRenderer::DestroyFramebuffers()
{
    for (uint32_t i = 0; i < vkFramebuffers.size(); i++)
        vkDestroyFramebuffer(vulkanDevice.vkDevice, vkFramebuffers[i], nullptr);
}

/**
 * @brief Depends on vertex input format, pipeline layout, shader, viewport, render pass.
 * 
 */
void VulkanRenderer::CreatePipelines()
{
    std::unique_ptr<VulkanPipeline> renderPipeline = 
        std::make_unique<VulkanPipeline>(vulkanDevice.vkDevice);
    PipelineLayoutBuilder layoutBuilder(&vulkanDevice);
    std::unique_ptr<VulkanPipelineLayout> pipelineLayout;


    renderPipeline->LoadShader("resources/vulkan_shaders/Phong/vert.spv",
                              "resources/vulkan_shaders/Phong/frag.spv");
    
    layoutBuilder.PushDescriptorSetLayout("material",
    {
        layoutBuilder.descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
        layoutBuilder.descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
    });

    layoutBuilder.PushDescriptorSetLayout("model",
    {
        layoutBuilder.descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0)
    });

    layoutBuilder.PushDescriptorSetLayout("camera",
    {
        layoutBuilder.descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0)
    });

    layoutBuilder.PushDescriptorSetLayout("scene",
    {
        layoutBuilder.descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
    });

    pipelineLayout = layoutBuilder.BuildPipelineLayout(vkDescriptorPool);

    renderPipeline->BuildPipeline(
        VulkanVertexbuffer::GetVertexInputState(),
        std::move(pipelineLayout),
        vkRenderPass.defaultCamera
    );

    pipelines["render"] = std::move(renderPipeline);
}

void VulkanRenderer::BeginFrame()
{
    imguiPlugin.BeginFrame();
}

void VulkanRenderer::EndFrame()
{
    VulkanCmdBuffer& vcb = vulkanCmdBuffer;

    VkCommandBuffer vkCommandBuffer = vcb.BeginCommand();
    VkSemaphore imageAcquiredSemaphore = vcb.GetCurrImageSemaphore();
    VkSemaphore renderFinishedSemaphore = vcb.GetCurrRenderSemaphore();

    int imageIndex = swapchain->GetNextImageIndex(imageAcquiredSemaphore);

    VkClearValue clearValue{{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkRenderPassBeginInfo vkRenderPassinfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    vkRenderPassinfo.renderPass = vkRenderPass.display;
    vkRenderPassinfo.framebuffer = vkFramebuffers[imageIndex];
    vkRenderPassinfo.renderArea.extent.height = swapchain->GetHeight();
    vkRenderPassinfo.renderArea.extent.width = swapchain->GetWidth();
    vkRenderPassinfo.clearValueCount = 1;
    vkRenderPassinfo.pClearValues = &clearValue;
    vkCmdBeginRenderPass(vkCommandBuffer, &vkRenderPassinfo, VK_SUBPASS_CONTENTS_INLINE);

    imguiPlugin.EndFrame(vkCommandBuffer);

    vkCmdEndRenderPass(vkCommandBuffer);

    DrawCamera(vkCommandBuffer);

    vcb.EndCommand();

    // Present image
    swapchain->PresentImage(renderFinishedSemaphore, imageIndex);
    commandQueue.clear();
}

void VulkanRenderer::DeallocateResources()
{
    //TODO: to match allocated resources 
    vkDeviceWaitIdle(vulkanDevice.vkDevice);
    swapchain->Destroy();
    vulkanCmdBuffer.Destroy();
}

void VulkanRenderer::Destroy()
{
    //DestroyPipelines();
    //TODO:
}

void VulkanRenderer::ExecuteRecordedCommands(VkCommandBuffer vkCommandBuffer)
{
    for(RenderCommand& command: commandQueue)
        command(vkCommandBuffer);
}

void VulkanRenderer::RecordCommand(RenderCommand renderCommand)
{
    commandQueue.push_back(renderCommand);
}

void VulkanRenderer::CleanupCommands()
{
    commandQueue.clear();
}

VulkanPipelineLayout& VulkanRenderer::GetPipelineLayout(std::string name)
{
    return *(pipelines[name]->pipelineLayout);
}

void VulkanRenderer::DrawCamera(VkCommandBuffer vkCommandBuffer)
{

    for (VulkanCamera* camera: cameraList)
    {
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{128/255.0, 204/255.0, 255/255.0, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo vkRenderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        vkRenderPassInfo.renderPass = vkRenderPass.defaultCamera;
        vkRenderPassInfo.framebuffer = camera->GetFrameBuffer();
        vkRenderPassInfo.renderArea.extent.height = static_cast<uint32_t>(camera->extent.y);
        vkRenderPassInfo.renderArea.extent.width = static_cast<uint32_t>(camera->extent.x);
        vkRenderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        vkRenderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(vkCommandBuffer, &vkRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(vkCommandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelines["render"]->pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<uint32_t>(camera->extent.x);
        viewport.height = static_cast<uint32_t>(camera->extent.y);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(vkCommandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {
            static_cast<uint32_t>(camera->extent.x),
            static_cast<uint32_t>(camera->extent.y)
        };
        vkCmdSetScissor(vkCommandBuffer, 0, 1, &scissor);

        camera->BindDescriptorSet(vkCommandBuffer,
            pipelines["render"]->pipelineLayout->layout);
        mainLight->BindDescriptorSet(vkCommandBuffer,
            pipelines["render"]->pipelineLayout->layout);

        ExecuteRecordedCommands(vkCommandBuffer);

        vkCmdEndRenderPass(vkCommandBuffer);
    }
}

void VulkanRenderer::AddCamera(VulkanCamera* vulkanCamera, glm::vec2 extent)
{
    vulkanCamera->Initialize(extent, swapchain->GetImageFormat());
    cameraList.push_back(vulkanCamera);

    //TODO: need to integrate IMGUI vulkan backend into this renderer
    VulkanTextureColor2D& colorImage = vulkanCamera->GetColorImage();
    vulkanCamera->cameraTexture = 
        reinterpret_cast<VkDescriptorSet>(
        imguiPlugin.MakeTexture(colorImage.vkSampler, colorImage.vkImageView,    
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
}  

void VulkanRenderer::RemoveCamera(VulkanCamera* vulkanCamera)
{
    vulkanCamera->Destroy();

    for(auto i = cameraList.begin(); i != cameraList.cend(); i++)
    {
        if (*i == vulkanCamera)
        {
            cameraList.erase(i);
            return;
        }
    }

    Log::Write(Log::Level::Error, "[Vulkan Render] Camera pointer not found.");
    throw ;
}

void VulkanRenderer::AddLight(VulkanLight* vulkanLight)
{
    mainLight = vulkanLight;
}