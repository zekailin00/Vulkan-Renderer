#include "imgui_plugin.h"

#include "vulkan_renderer.h"
#include "validation.h"

void ImguiPlugin::InitializePlugin(ImGui_ImplVulkan_InitInfo& imguiVulkanInitInfo, VkRenderPass vkRenderPass)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGui_ImplVulkan_Init(&imguiVulkanInitInfo, vkRenderPass);

    // Create font
    io.Fonts->AddFontFromFileTTF("resources/KGPrimaryPenmanship.ttf", 18.0f);

    VulkanSingleCmd cmd{};
    cmd.Initialize(&renderer::VulkanRenderer::GetInstance().vulkanDevice);
    VkCommandBuffer vkCommandBuffer = cmd.BeginCommand();
    
    ImGui_ImplVulkan_CreateFontsTexture(vkCommandBuffer);
    cmd.EndCommand();

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImguiPlugin::BeginFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
}

ImTextureID ImguiPlugin::MakeTexture(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
{
    return reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(sampler, imageView, imageLayout));
}

void ImguiPlugin::EndFrame(VkCommandBuffer vkCommandBuffer)
{
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, vkCommandBuffer);
}

void ImguiPlugin::DestroyPlugin()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui::DestroyContext();
}