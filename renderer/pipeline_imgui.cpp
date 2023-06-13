#include "pipeline_imgui.h"

#include "vk_primitives/vulkan_pipeline.h"
#include "vk_primitives/vulkan_pipeline_layout.h"
#include "vk_primitives/vulkan_device.h"
#include "validation.h"

#include <imgui.h>
#include <memory>
#include <array>
#include <tracy/Tracy.hpp>


namespace renderer
{

PipelineImgui::PipelineImgui(
    VulkanDevice& vulkanDevice, VkDescriptorPool vkDescriptorPool, VkRenderPass renderpass)
{
    ZoneScopedN("PipelineImgui::PipelineImgui");

    this->vulkanDevice = vulkanDevice;
    this->vkDescriptorPool = vkDescriptorPool;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Enablde docking
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    // Setup backend capabilities flags
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.BackendRendererUserData = static_cast<void*>(this);
    io.BackendRendererName = "Vulkan-Renderer";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    // Create font
    io.Fonts->AddFontFromFileTTF("resources/KGPrimaryPenmanship.ttf", 20.0f);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    {   // Build imgui pipeline
        imguiPipeline = std::make_unique<VulkanPipeline>(vulkanDevice.vkDevice);
        PipelineLayoutBuilder layoutBuilder(&vulkanDevice);
        std::unique_ptr<VulkanPipelineLayout> imguiLayout;

        imguiPipeline->LoadShader("resources/vulkan_shaders/imgui/vert.spv",
                                  "resources/vulkan_shaders/imgui/frag.spv");
        
        layoutBuilder.PushDescriptorSetLayout("fontTexture",
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
        pipelineLayout.AllocateDescriptorSet("fontTexture", 1, &fontTextureDescSet);
        
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

    io.Fonts->SetTexID(reinterpret_cast<ImTextureID>(fontTextureDescSet));
}

PipelineImgui::~PipelineImgui()
{
    ZoneScopedN("PipelineImgui::~PipelineImgui");

    vkDeviceWaitIdle(vulkanDevice.vkDevice);
    ImGuiIO& io = ImGui::GetIO();

    vkFreeDescriptorSets(vulkanDevice.vkDevice, vkDescriptorPool, 1, &fontTextureDescSet);
    fontTexture->Destroy();
    imguiPipeline = nullptr;

    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    ImGui::DestroyContext();
}


void PipelineImgui::RenderUI(ImDrawData* drawData,
    VkBuffer vertexBuffer, VkBuffer indexBuffer, VkCommandBuffer commandbuffer)
{
    ZoneScopedN("PipelineImgui::RenderUI");
    
    // Avoid rendering when minimized, scale coordinates 
    // for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    int fb_height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;
    
    // Bind pipeline
    {
        vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, imguiPipeline->pipeline);
    }

    // Bind Vertex And Index Buffer:
    if (drawData->TotalVtxCount > 0)
    {
        VkBuffer vertex_buffers[1] = { vertexBuffer };
        VkDeviceSize vertex_offset[1] = { 0 };
        vkCmdBindVertexBuffers(commandbuffer, 0, 1, vertex_buffers, vertex_offset);
        vkCmdBindIndexBuffer(commandbuffer, indexBuffer, 0,
            sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
    }

    // Setup viewport
    {
        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = (float)fb_width;
        viewport.height = (float)fb_height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandbuffer, 0, 1, &viewport);
    }

    // Setup scale and translation
    // Our visible imgui space lies from draw_data->DisplayPps (top left)
    // to draw_data->DisplayPos+data_data->DisplaySize (bottom right). 
    // DisplayPos is (0,0) for single viewport apps.
    {
        float scale[2];
        scale[0] = 2.0f / drawData->DisplaySize.x;
        scale[1] = 2.0f / drawData->DisplaySize.y;
        float translate[2];
        translate[0] = -1.0f - drawData->DisplayPos.x * scale[0];
        translate[1] = -1.0f - drawData->DisplayPos.y * scale[1];
        vkCmdPushConstants(commandbuffer, imguiPipeline->pipelineLayout->layout,
            VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 0, sizeof(float) * 2, scale);
        vkCmdPushConstants(commandbuffer, imguiPipeline->pipelineLayout->layout,
            VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);
    }

    // Will project scissor/clipping rectangles into framebuffer space
    // (0,0) unless using multi-viewports
    ImVec2 clip_off = drawData->DisplayPos;
    // (1,1) unless using retina display which are often (2,2)
    ImVec2 clip_scale = drawData->FramebufferScale;

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_vtx_offset = 0;
    int global_idx_offset = 0;
    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = drawData->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            
            // Project scissor/clipping rectangles into framebuffer space
            ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
            ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

            // Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
            if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
            if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
            if (clip_max.x > fb_width) { clip_max.x = (float)fb_width; }
            if (clip_max.y > fb_height) { clip_max.y = (float)fb_height; }
            if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                continue;

            // Apply scissor/clipping rectangle
            VkRect2D scissor;
            scissor.offset.x = (int32_t)(clip_min.x);
            scissor.offset.y = (int32_t)(clip_min.y);
            scissor.extent.width = (uint32_t)(clip_max.x - clip_min.x);
            scissor.extent.height = (uint32_t)(clip_max.y - clip_min.y);
            vkCmdSetScissor(commandbuffer, 0, 1, &scissor);

            // Bind DescriptorSet with font or user texture
            VkDescriptorSet desc_set[1] = { (VkDescriptorSet)pcmd->TextureId };
            if (sizeof(ImTextureID) < sizeof(ImU64))
            {
                desc_set[0] = fontTextureDescSet;
            }
            vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                imguiPipeline->pipelineLayout->layout, 0, 1, desc_set, 0, nullptr);

            // Draw
            vkCmdDrawIndexed(commandbuffer, pcmd->ElemCount, 1,
                pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
        
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }

    VkRect2D scissor = { { 0, 0 }, { (uint32_t)fb_width, (uint32_t)fb_height } };
    vkCmdSetScissor(commandbuffer, 0, 1, &scissor);
}

} // namespace renderer
