#include "render_technique.h"

#include "vulkan_mesh.h"
#include "vulkan_renderer.h"
#include "vk_primitives/vulkan_pipeline_layout.h"

#include <glm/glm.hpp>
#include <array>
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
        return;

    VulkanRenderer& vkr = VulkanRenderer::GetInstance();
    VkPipelineLayout layout = vkr.GetPipelineLayout("render").layout;

    for (std::shared_ptr<VulkanCamera> camera: cameraList)
    {
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

void RenderTechnique::DrawCamera(VkCommandBuffer vkCommandBuffer)
{

}

} // namespace renderer
