#include "pipeline_imgui.h"

#include "vk_primitives/vulkan_pipeline.h"
#include "vk_primitives/vulkan_pipeline_layout.h"
#include "vk_primitives/vulkan_device.h"

#include <imgui.h>
#include <memory>
#include <array>


namespace renderer
{

PipelineImgui::PipelineImgui(
    VulkanDevice& vulkanDevice, VkDescriptorPool vkDescriptorPool, VkRenderPass renderpass)
{
    vkDevice = vulkanDevice.vkDevice;
    vkDescriptorPool = vkDescriptorPool;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Setup backend capabilities flags
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.BackendRendererUserData = static_cast<void*>(this);
    io.BackendRendererName = "Vulkan-Renderer";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    // Create font
    io.Fonts->AddFontFromFileTTF("resources/KGPrimaryPenmanship.ttf", 18.0f);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    {   // Build imgui pipeline
        imguiPipeline = std::make_unique<VulkanPipeline>(vulkanDevice.vkDevice);
        PipelineLayoutBuilder layoutBuilder(&vulkanDevice);
        std::unique_ptr<VulkanPipelineLayout> imguiLayout;

        imguiPipeline->LoadShader("resources/vulkan_shaders/imgui/vert.spv",
                                  "resources/vulkan_shaders/imgui/frag.spv");
        
        layoutBuilder.PushDescriptorSetLayout("font",
        {
            layoutBuilder.descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
        });

        VkPushConstantRange range = {};
        range.offset = 0;
        range.size = sizeof(ImguiPushConst);
        range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        imguiLayout = layoutBuilder.BuildPipelineLayout(
            vkDescriptorPool,
            &range
        );

        // Extra pipeline configurations for imgui rendering
        // Enable alpha blending and disable depth testing
        imguiPipeline->rasterState.cullMode = VK_CULL_MODE_NONE;
        imguiPipeline->blendAttachment.blendEnable = VK_TRUE;
        imguiPipeline->blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        imguiPipeline->blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        imguiPipeline->blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        imguiPipeline->blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        imguiPipeline->depthStencilState = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};


        // Vertex input configurations
        VkVertexInputBindingDescription binding_desc[1] = {};
        binding_desc[0].stride = sizeof(ImDrawVert);
        binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription attribute_desc[3] = {};
        attribute_desc[0].location = 0;
        attribute_desc[0].binding = binding_desc[0].binding;
        attribute_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_desc[0].offset = IM_OFFSETOF(ImDrawVert, pos);
        attribute_desc[1].location = 1;
        attribute_desc[1].binding = binding_desc[0].binding;
        attribute_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_desc[1].offset = IM_OFFSETOF(ImDrawVert, uv);
        attribute_desc[2].location = 2;
        attribute_desc[2].binding = binding_desc[0].binding;
        attribute_desc[2].format = VK_FORMAT_R8G8B8A8_UNORM;
        attribute_desc[2].offset = IM_OFFSETOF(ImDrawVert, col);

        VkPipelineVertexInputStateCreateInfo vertex_info = {};
        vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_info.vertexBindingDescriptionCount = 1;
        vertex_info.pVertexBindingDescriptions = binding_desc;
        vertex_info.vertexAttributeDescriptionCount = 3;
        vertex_info.pVertexAttributeDescriptions = attribute_desc;

        imguiPipeline->BuildPipeline(
            &vertex_info,
            std::move(imguiLayout),
            renderpass
        );
    }

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    TextureBuildInfo info{};
    fontTexture = std::dynamic_pointer_cast<VulkanTexture>(
        VulkanTexture::BuildTextureFromBuffer(pixels, width, height, &info));
        
    {   // Create font texture descriptor set
        VulkanPipelineLayout& pipelineLayout = *(imguiPipeline->pipelineLayout);
        pipelineLayout.AllocateDescriptorSet("texture", 1, &fontTextureDescSet);
        
        std::array<VkWriteDescriptorSet, 1> descriptorWrite{};

        descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[0].dstSet = fontTextureDescSet;
        descriptorWrite[0].dstBinding = 0;
        descriptorWrite[0].dstArrayElement = 0;
        descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite[0].descriptorCount = 1;
        descriptorWrite[0].pImageInfo = fontTexture->GetDescriptor();

        vkUpdateDescriptorSets(vulkanDevice.vkDevice,
            descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
    }

    io.Fonts->SetTexID(static_cast<ImTextureID>(fontTextureDescSet));
}

PipelineImgui::~PipelineImgui()
{
    vkDeviceWaitIdle(vkDevice);
    ImGuiIO& io = ImGui::GetIO();

    vkFreeDescriptorSets(vkDevice, vkDescriptorPool, 1, &fontTextureDescSet);
    fontTexture->Destroy();
    imguiPipeline = nullptr;

    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    ImGui::DestroyContext();
}


void PipelineImgui::RenderUI(ImDrawData* drawData, VkCommandBuffer commandbuffer)
{

}

} // namespace renderer
