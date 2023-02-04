#pragma once 

#include "imgui.h"
#include "imgui_impl_vulkan.h"


class ImguiPlugin
{
public:
    void InitializePlugin(ImGui_ImplVulkan_InitInfo& imguiVulkanInitInfo, VkRenderPass vkRenderPass);
    void BeginFrame();
    void EndFrame(VkCommandBuffer vkCommandBuffer);
    void DestroyPlugin();

    ImTextureID MakeTexture(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout);
};