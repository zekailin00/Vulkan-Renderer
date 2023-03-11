#include "render_technique.h"

#include "vulkan_mesh.h"
#include "vulkan_renderer.h"
#include "vulkan_texture.h"
#include "vk_primitives/vulkan_pipeline_layout.h"

#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <memory>

namespace renderer
{

void RenderTechnique::ProcessScene(VulkanNode* root)
{
    glm::mat4 identity = glm::mat4(1);
    sceneMap->nLight = 0;
    ScanNode(root, identity);

    if(sceneMap->nLight == 0)
    {
        sceneMap->dirLight[0] = VulkanLight::GetDefaultLight()->dirLight;
        sceneMap->nLight++;
    }
}

void RenderTechnique::ExecuteCommand(VkCommandBuffer commandBuffer)
{
    if(cameraList.empty())
    {
        this->display = defaultDisplay;
        return;
    }

    VulkanRenderer& vkr = VulkanRenderer::GetInstance();
    VkPipelineLayout layout = vkr.GetPipelineLayout("render").layout;

    std::vector<VkImageMemoryBarrier> barriers;
    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    if (!cameraList.empty())
        this->display = cameraList[0]->colorTexDescSet;


    for (std::shared_ptr<VulkanCamera> camera: cameraList)
    {
        barrier.image = camera->colorImage.GetImage();
        barriers.push_back(barrier);

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0/255.0, 0/255.0, 0/255.0, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo vkRenderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        vkRenderPassInfo.renderPass = vkr.vkRenderPass.defaultCamera;
        vkRenderPassInfo.framebuffer = camera->GetFrameBuffer();
        vkRenderPassInfo.renderArea.extent = {
            camera->GetCamProperties().Extent.x,
            camera->GetCamProperties().Extent.y
        };
        vkRenderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        vkRenderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(commandBuffer, &vkRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            vkr.GetPipeline("render").pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = camera->GetCamProperties().Extent.x;
        viewport.height = camera->GetCamProperties().Extent.y;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {
            camera->GetCamProperties().Extent.x,
            camera->GetCamProperties().Extent.y
        };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // FIXME: bind location
        vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
            layout, 2, 1, camera->GetDescriptorSet(), 0, nullptr
        );
        vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
            layout, 3, 1, &sceneDescSet, 0, nullptr
        );

        // ExecuteRecordedCommands(vkCommandBuffer);

        for(const auto& m: renderMesh)
        {
            VulkanVertexbuffer& vvb = m.mesh->GetVertexbuffer();
            VulkanMaterial& vm = m.mesh->GetVulkanMaterial();

            vkCmdBindDescriptorSets(
                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                layout, 1, 1, &m.descSet, 0, nullptr
            );

            vkCmdBindDescriptorSets(
                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                layout, 0, 1, vm.GetDescriptorSet(), 0, nullptr
            );

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vvb.vertexBuffer, &offset);
            vkCmdBindIndexBuffer(commandBuffer, vvb.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, vvb.GetIndexCount(), 1, 0, 0, 0);
        }


        vkCmdEndRenderPass(commandBuffer);
    }

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr,
        barriers.size(), barriers.data() 
    );
}

void RenderTechnique::ScanNode(VulkanNode* node, const glm::mat4& transform)
{
    glm::mat4 globTransform = node->GetTransform() * transform;
    *node->transform = globTransform;

    if (node->mesh)
    {
        MeshPacket packet
        {
            std::dynamic_pointer_cast<VulkanMesh>(node->mesh),
            node->descSet
        };

        renderMesh.push_back(packet);
    }
    else if (node->light)
    {
        std::shared_ptr<VulkanLight> light = 
            std::dynamic_pointer_cast<VulkanLight>(node->light);
        light->SetTransform(globTransform);

        if (sceneMap->nLight != 5)
        {
            uint32_t i = sceneMap->nLight++;
            sceneMap->dirLight[i] = light->dirLight;
        }
    }
    else if (node->camera)
    {
        std::shared_ptr<VulkanCamera> vkCamera =
            std::dynamic_pointer_cast<VulkanCamera>(node->camera);
        vkCamera->SetTransform(globTransform);

        cameraList.push_back(std::dynamic_pointer_cast<VulkanCamera>(node->camera));
    }
    
    for (const auto& n: node->nodeLists)
    {
        ScanNode(static_cast<VulkanNode*>(n.get()), globTransform);
    }
}

void RenderTechnique::Initialize(VulkanDevice* vulkanDevice)
{
    VulkanRenderer& vkr = VulkanRenderer::GetInstance();
    sceneUniform.Initialize(vulkanDevice, sizeof(SceneData) * 5);
    sceneMap = static_cast<SceneData*>(sceneUniform.Map());
    
    {
        VulkanPipelineLayout& layout = vkr.GetPipelineLayout("render");
        layout.AllocateDescriptorSet("scene", vkr.FRAME_IN_FLIGHT, &sceneDescSet);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = this->sceneDescSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = this->sceneUniform.GetDescriptor();

        vkUpdateDescriptorSets(
            vulkanDevice->vkDevice, 1, &descriptorWrite, 0, nullptr);
    }

    {
        VulkanPipelineLayout& layout = vkr.GetPipelineLayout("display");
        layout.AllocateDescriptorSet("texture", vkr.FRAME_IN_FLIGHT, &defaultDisplay);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = this->defaultDisplay;
        descriptorWrite.dstBinding = 1;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = VulkanTexture::GetDefaultTexture()->GetDescriptor();

        vkUpdateDescriptorSets(
            vulkanDevice->vkDevice, 1, &descriptorWrite, 0, nullptr);
    }
}

void RenderTechnique::DrawCamera(VkCommandBuffer vkCommandBuffer)
{

}

} // namespace renderer
