#include "vulkan_renderer.h"

#include "vulkan_swapchain.h"
#include "vulkan_texture.h"
#include "vulkan_scene.h"
#include "vulkan_wireframe.h"

#include "vk_primitives/vulkan_vertexbuffer.h"
#include "vk_primitives/vulkan_device.h"
#include "vk_primitives/vulkan_cmdbuffer.h"
#include "vk_primitives/vulkan_shader.h"
#include "vk_primitives/vulkan_pipeline.h"

#include "validation.h"
#include "logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <vector>
#include <array>
#include <memory>
#include <stddef.h> // offset(type, member)

#include <Tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

#define VULKAN_DEBUG_REPORT
// #define VULKAN_APPLE_SUPPORT

#ifdef VULKAN_DEBUG_REPORT
static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(
    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
    uint64_t object, size_t location, int32_t messageCode,
    const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    Logger::Write(
        "[Vulkan Renderer] Debug report from ObjectType: " +
        std::to_string(objectType) + " at location "+
        std::to_string(location) + ".\n\tMessage: " + pMessage + "\n",
        Logger::Level::Warning,
        Logger::MsgType::Renderer
    );

    return VK_FALSE;
}
#endif


namespace renderer
{

TracyVkCtx tracyVkCtx;

void VulkanRenderer::InitializeDevice(
    std::vector<const char *> instanceExt, std::vector<const char *> deviceExt)
{
    ZoneScopedN("VulkanRenderer::InitializeDevice");

    VkResult err;

    // Create Vulkan Instance
    {
        std::vector<const char*> extensionList = instanceExt;

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
        auto vkCreateDebugReportCallbackEXT =
            (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");
        ASSERT(vkCreateDebugReportCallbackEXT != NULL);

        // Setup the debug report callback
        VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
        debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
            VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
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

    vulkanDevice.Initialize(vkInstance, deviceExt);

    {   // Tracy Vulkan context
        VulkanSingleCmd singleCmd;
        singleCmd.Initialize(&vulkanDevice);
        VkCommandBuffer commandbuffer = singleCmd.BeginCommand();
        singleCmd.EndCommand();
        tracyVkCtx = TracyVkContext(vulkanDevice.vkPhysicalDevice, vulkanDevice.vkDevice,
            vulkanDevice.graphicsQueue, commandbuffer);
    }
}

void VulkanRenderer::AllocateResources(
    IVulkanSwapchain* glfwSwapchain, IVulkanSwapchain* openxrSwapchain)
{
    ZoneScopedN("VulkanRenderer::AllocateResources");

    this->swapchain = glfwSwapchain;
    swapchain->Initialize(&vulkanDevice);
    
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
    defaultTechnique.Initialize(&vulkanDevice);
}

void VulkanRenderer::RebuildSwapchain()
{
    ZoneScopedN("VulkanRenderer::RebuildSwapchain");

    vkDeviceWaitIdle(vulkanDevice.vkDevice);

    DestroyFramebuffers();
    swapchain->RebuildSwapchain(&vulkanDevice);
    CreateFramebuffers();
}

void VulkanRenderer::CreateRenderPasses()
{
    ZoneScopedN("VulkanRenderer::CreateRenderPasses");

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


    // imgui render pass
    {
        std::vector<VkAttachmentDescription> attachmentDesc{colorAttachmentDesc};

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachment;

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
        vkRenderPassCreateInfo.pAttachments = attachmentDesc.data();
        vkRenderPassCreateInfo.subpassCount = 1;
        vkRenderPassCreateInfo.pSubpasses = &subpass;
        vkRenderPassCreateInfo.dependencyCount = 1;
        vkRenderPassCreateInfo.pDependencies = &dependency;

        CHECK_VKCMD(vkCreateRenderPass(vulkanDevice.vkDevice, &vkRenderPassCreateInfo, nullptr, &vkRenderPass.imgui));
    }

}

void VulkanRenderer::DestroyRenderPasses()
{
    ZoneScopedN("VulkanRenderer::DestroyRenderPasses");

    vkDestroyRenderPass(vulkanDevice.vkDevice, vkRenderPass.defaultCamera, nullptr);
    vkDestroyRenderPass(vulkanDevice.vkDevice, vkRenderPass.display, nullptr);
    vkDestroyRenderPass(vulkanDevice.vkDevice, vkRenderPass.imgui, nullptr);
}

void VulkanRenderer::CreateFramebuffers()
{
    ZoneScopedN("VulkanRenderer::CreateFramebuffers");

    VkImageView attachment[1];
    VkFramebufferCreateInfo vkFramebufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    vkFramebufferCreateInfo.renderPass = vkRenderPass.display;
    vkFramebufferCreateInfo.attachmentCount = 1;
    vkFramebufferCreateInfo.pAttachments = attachment;
    vkFramebufferCreateInfo.width = swapchain->GetWidth();
    vkFramebufferCreateInfo.height = swapchain->GetHeight();
    vkFramebufferCreateInfo.layers = 1;
    for (uint32_t i = 0; i < swapchain->GetImageCount(); i++)
    {
        attachment[0] = swapchain->GetImageView(i);
        CHECK_VKCMD(vkCreateFramebuffer(
            vulkanDevice.vkDevice, &vkFramebufferCreateInfo,nullptr, swapchain->GetFramebuffer(i)));
    }
}

void VulkanRenderer::DestroyFramebuffers()
{
    ZoneScopedN("VulkanRenderer::DestroyFramebuffers");

    for (uint32_t i = 0; i < swapchain->GetImageCount(); i++)
        vkDestroyFramebuffer(vulkanDevice.vkDevice, *swapchain->GetFramebuffer(i), nullptr);
}

/**
 * @brief Depends on vertex input format, pipeline layout, shader, viewport, render pass.
 * 
 */
void VulkanRenderer::CreatePipelines()
{
    ZoneScopedN("VulkanRenderer::CreatePipelines");

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
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
            layoutBuilder.descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
            layoutBuilder.descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
            layoutBuilder.descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4),
        });

        layoutBuilder.PushDescriptorSetLayout("mesh",
        {
            /*
            layout (set = 1, binding = 0) uniform MeshCoordinates
            {
                mat4 model;
            } m;
            */
            layoutBuilder.descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0)
        });

        layoutBuilder.PushDescriptorSetLayout("camera",
        {
            /*
            layout (set = 2, binding = 0) uniform ViewProjection 
            {
                mat4 view;
                mat4 projection;
            } vp;
            */
            layoutBuilder.descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0)
        });

        layoutBuilder.PushDescriptorSetLayout("scene",
        {
            layoutBuilder.descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
        });

        pipelineLayout = layoutBuilder.BuildPipelineLayout(vkDescriptorPool);
        renderPipeline->rasterState.frontFace = VK_FRONT_FACE_CLOCKWISE;

        renderPipeline->BuildPipeline(
            VulkanVertexbuffer::GetVertexInputState(),
            std::move(pipelineLayout),
            vkRenderPass.defaultCamera
        );

        pipelines["render"] = std::move(renderPipeline);
    }

    {
        std::unique_ptr<VulkanPipeline> quadPipeline = 
            std::make_unique<VulkanPipeline>(vulkanDevice.vkDevice);
        PipelineLayoutBuilder layoutBuilder(&vulkanDevice);
        std::unique_ptr<VulkanPipelineLayout> quadLayout;

        quadPipeline->LoadShader("resources/vulkan_shaders/quad/vert.spv",
                                "resources/vulkan_shaders/quad/frag.spv");
        
        layoutBuilder.PushDescriptorSetLayout("texture",
        {
            layoutBuilder.descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
        });

        quadLayout = layoutBuilder.BuildPipelineLayout(vkDescriptorPool);

        VkPipelineVertexInputStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        info.vertexAttributeDescriptionCount = 0;
        info.vertexBindingDescriptionCount = 0;

        quadPipeline->rasterState.cullMode = VK_CULL_MODE_BACK_BIT;
        quadPipeline->rasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        quadPipeline->BuildPipeline(
            &info,
            std::move(quadLayout),
            vkRenderPass.display
        );

        pipelines["display"] = std::move(quadPipeline);
    }

    {
        std::unique_ptr<VulkanPipeline> skyboxPipeline = 
            std::make_unique<VulkanPipeline>(vulkanDevice.vkDevice);
        PipelineLayoutBuilder layoutBuilder(&vulkanDevice);
        std::unique_ptr<VulkanPipelineLayout> skyboxLayout;

        skyboxPipeline->LoadShader("resources/vulkan_shaders/skybox/vert.spv",
                                    "resources/vulkan_shaders/skybox/frag.spv");
        
        layoutBuilder.PushDescriptorSetLayout("textureCube",
        {
            layoutBuilder.descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
        });

        layoutBuilder.PushDescriptorSetLayout("camera",
        {
            /*
            layout (set = 0, binding = 0) uniform ViewProjection 
            {
                mat4 view;
                mat4 projection;
            } vp;
            */
            layoutBuilder.descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0)
        });

        skyboxLayout = layoutBuilder.BuildPipelineLayout(vkDescriptorPool);

        skyboxPipeline->rasterState.cullMode = VK_CULL_MODE_NONE;
        skyboxPipeline->rasterState.frontFace = VK_FRONT_FACE_CLOCKWISE;
        skyboxPipeline->depthStencilState.depthTestEnable = VK_FALSE;

        skyboxPipeline->BuildPipeline(
            VulkanVertexbuffer::GetVertexInputState(),
            std::move(skyboxLayout),
            vkRenderPass.defaultCamera
        );

        pipelines["skybox"] = std::move(skyboxPipeline);
    }

    {
        std::unique_ptr<VulkanPipeline> wirePipeline = 
            std::make_unique<VulkanPipeline>(vulkanDevice.vkDevice);
        PipelineLayoutBuilder layoutBuilder(&vulkanDevice);
        std::unique_ptr<VulkanPipelineLayout> wireLayout;

        wirePipeline->LoadShader("resources/vulkan_shaders/wire/vert.spv",
                                 "resources/vulkan_shaders/wire/frag.spv");
        
        layoutBuilder.PushDescriptorSetLayout("camera",
        {
            /*
            layout (set = 0, binding = 0) uniform ViewProjection 
            {
                mat4 view;
                mat4 projection;
            } vp;
            */
            layoutBuilder.descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0)
        });

        VkPushConstantRange range = {};
        range.offset = 0;
        range.size = sizeof(WirePushConst);
        range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        wireLayout = layoutBuilder.BuildPipelineLayout(
            vkDescriptorPool,
            &range
        );

        wirePipeline->rasterState.cullMode = VK_CULL_MODE_NONE;

        VkPipelineVertexInputStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        info.vertexAttributeDescriptionCount = 0;
        info.vertexBindingDescriptionCount = 0;

        wirePipeline->BuildPipeline(
            &info,
            std::move(wireLayout),
            vkRenderPass.defaultCamera
        );

        pipelines["wire"] = std::move(wirePipeline);
    }

    pipelineImgui = std::make_unique<PipelineImgui>(
        vulkanDevice, vkDescriptorPool, vkRenderPass.imgui);
}

void VulkanRenderer::BeginFrame()
{
    ZoneScopedN("VulkanRenderer::BeginFrame");
    // Nothing
}

void VulkanRenderer::EndFrame()
{
    ZoneScopedN("VulkanRenderer::EndFrame");

    if (scene)
        defaultTechnique.ProcessScene(static_cast<VulkanNode*>(scene->GetRootNode()));

    VulkanCmdBuffer& vcb = vulkanCmdBuffer;

    VkCommandBuffer vkCommandBuffer = vcb.BeginCommand();
    VkSemaphore imageAcquiredSemaphore = vcb.GetCurrImageSemaphore();
    VkSemaphore renderFinishedSemaphore = vcb.GetCurrRenderSemaphore();

    int imageIndex = swapchain->GetNextImageIndex(&vulkanDevice, imageAcquiredSemaphore);

    defaultTechnique.ExecuteCommand(vkCommandBuffer);

    {
        TracyVkZone(tracyVkCtx, vkCommandBuffer, "EndFrame#Display");

        // Initialize swapchain image
        VkClearValue clearValue{{{0.0f, 0.0f, 0.0f, 1.0f}}};
        VkRenderPassBeginInfo vkRenderPassinfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        vkRenderPassinfo.renderPass = vkRenderPass.display;
        vkRenderPassinfo.framebuffer = *swapchain->GetFramebuffer(imageIndex);
        vkRenderPassinfo.renderArea.extent.height = swapchain->GetHeight();
        vkRenderPassinfo.renderArea.extent.width = swapchain->GetWidth();
        vkRenderPassinfo.clearValueCount = 1;
        vkRenderPassinfo.pClearValues = &clearValue;
        vkCmdBeginRenderPass(vkCommandBuffer, &vkRenderPassinfo, VK_SUBPASS_CONTENTS_INLINE);

        VkDescriptorSet descSet = defaultTechnique.GetDisplayDescSet();

        vkCmdBindPipeline(vkCommandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            GetPipeline("display").pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = swapchain->GetWidth();
        viewport.height = swapchain->GetHeight();
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(vkCommandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {
            swapchain->GetWidth(), 
            swapchain->GetHeight()
        };
        vkCmdSetScissor(vkCommandBuffer, 0, 1, &scissor);

        vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelines["display"]->pipelineLayout->layout,
            0, 1, &descSet, 0, nullptr);

        vkCmdDraw(vkCommandBuffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(vkCommandBuffer);
    }

    TracyVkCollect(tracyVkCtx, vkCommandBuffer);
    vcb.EndCommand();

    // Present image
    swapchain->PresentImage(&vulkanDevice, renderFinishedSemaphore, imageIndex);
}

void VulkanRenderer::DeallocateResources()
{
    ZoneScopedN("VulkanRenderer::DeallocateResources");

    this->scene.reset();

    vkDeviceWaitIdle(vulkanDevice.vkDevice);

    this->defaultTechnique.Destroy();

    VulkanMaterial::DestroyDefaultMaterial();
    VulkanTexture::DestroyDefaultTexture();
    VulkanTextureCube::DestroyDefaultTexture();

    DestroyFramebuffers();
    swapchain->Destroy(&vulkanDevice);

    pipelines.clear();
    pipelineImgui.reset();
    DestroyRenderPasses();

    vulkanCmdBuffer.Destroy();
    vkDestroyDescriptorPool(vulkanDevice.vkDevice, vkDescriptorPool, nullptr);
}

void VulkanRenderer::Destroy()
{
    ZoneScopedN("VulkanRenderer::Destroy");

    TracyVkDestroy(tracyVkCtx);
    vulkanDevice.Destroy();

#ifdef VULKAN_DEBUG_REPORT

    auto vkDestroyDebugReportCallbackEXT = 
        (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");

    assert(vkDestroyDebugReportCallbackEXT);
    vkDestroyDebugReportCallbackEXT(vkInstance, g_DebugReport, nullptr);

#endif

    // Destory VkSurfaceKHR and sdebugUtilsMessengerEXT before vkInstance
    vkDestroyInstance(vkInstance, nullptr);
}

void VulkanRenderer::InitializeXrSession(IVulkanSwapchain* xrSwapchain)
{
    xrSwapchain->Initialize(&vulkanDevice);
    openxrSwapchain = xrSwapchain;


}

void VulkanRenderer::DestroyXrSession()
{
    openxrSwapchain->Destroy(&vulkanDevice);
    delete openxrSwapchain;
}

VulkanPipelineLayout& VulkanRenderer::GetPipelineLayout(std::string name)
{
    ZoneScopedN("VulkanRenderer::GetPipelineLayout");

    return *(pipelines[name]->pipelineLayout);
}

VulkanPipeline& VulkanRenderer::GetPipeline(std::string name)
{
    ZoneScopedN("VulkanRenderer::GetPipeline");

    return *(pipelines[name].get());
}

Scene* VulkanRenderer::CreateScene()
{
    ZoneScopedN("VulkanRenderer::CreateScene");

    this->scene = std::make_unique<VulkanScene>();
    return &(*this->scene);
}

Scene* VulkanRenderer::GetScene()
{
    ZoneScopedN("VulkanRenderer::GetScene");

    if (!scene)
        throw;
    return &(*this->scene);
}

} // namespace renderer