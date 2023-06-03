#include "render_technique.h"

#include "vulkan_mesh.h"
#include "vulkan_renderer.h"
#include "vulkan_texture.h"
#include "vulkan_wireframe.h"
#include "vulkan_ui.h"

#include "vk_primitives/vulkan_pipeline_layout.h"
#include "loaders/gltfloader.h"
#include "input.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <array>
#include <vector>
#include <memory>
#include <tracy/TracyVulkan.hpp>


// Creates a projection matrix based on the specified FOV.
inline static void XrProjectionFov(
    glm::mat4& result, glm::vec4 fov, float nearZ, float farZ)
{
    const float tanAngleLeft = tanf(fov[0]);
    const float tanAngleRight = tanf(fov[1]);

    const float tanAngleDown = tanf(fov[3]);
    const float tanAngleUp = tanf(fov[2]);

    const float tanAngleWidth = tanAngleRight - tanAngleLeft;
    const float tanAngleHeight = tanAngleUp - tanAngleDown;

    // Set to zero for a [0,1] Z clip space (Vulkan).
    const float offsetZ = 0;

    if (farZ <= nearZ) {
        // place the far plane at infinity
        result[0][0] = 2.0f / tanAngleWidth;
        result[1][0] = 0.0f;
        result[2][0] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
        result[3][0] = 0.0f;

        result[0][1] = 0.0f;
        result[1][1] = 2.0f / tanAngleHeight;
        result[2][1] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
        result[3][1] = 0.0f;

        result[0][2] = 0.0f;
        result[1][2] = 0.0f;
        result[2][2] = -1.0f;
        result[3][2] = -(nearZ + offsetZ);

        result[0][3] = 0.0f;
        result[1][3] = 0.0f;
        result[2][3] = -1.0f;
        result[3][3] = 0.0f;
    } else {
        // normal projection
        result[0][0] = 2.0f / tanAngleWidth;
        result[1][0] = 0.0f;
        result[2][0] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
        result[3][0] = 0.0f;

        result[0][1] = 0.0f;
        result[1][1] = 2.0f / tanAngleHeight;
        result[2][1] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
        result[3][1] = 0.0f;

        result[0][2] = 0.0f;
        result[1][2] = 0.0f;
        result[2][2] = -(farZ + offsetZ) / (farZ - nearZ);
        result[3][2] = -(farZ * (nearZ + offsetZ)) / (farZ - nearZ);

        result[0][3] = 0.0f;
        result[1][3] = 0.0f;
        result[2][3] = -1.0f;
        result[3][3] = 0.0f;
    }
}

namespace renderer
{

extern TracyVkCtx tracyVkCtx;

RenderTechnique::~RenderTechnique()
{
    ZoneScopedN("RenderTechnique::~RenderTechnique");

    Destroy();
}

void RenderTechnique::Destroy()
{
    ZoneScopedN("RenderTechnique::Destroy");

    sceneUniform.Destroy();
    skyboxMesh = nullptr;
    textureCube = nullptr;
}

void RenderTechnique::ProcessScene(VulkanNode* root)
{
    ZoneScopedN("RenderTechnique::ProcessScene");

    glm::mat4 identity = glm::mat4(1);
    sceneMap->nLight.x = 0;
    ScanNode(root, identity);

    if(sceneMap->nLight.x == 0)
    {
        sceneMap->dirLight[0] = VulkanLight::GetDefaultLight()->dirLight;
        sceneMap->nLight++;
    }
}

void RenderTechnique::ExecuteCommand(VkCommandBuffer commandBuffer)
{
    ZoneScopedN("RenderTechnique::ExecuteCommand");

    if(cameraList.empty())
    {
        this->display = defaultDisplay;
        return;
    }

    VulkanRenderer& vkr = VulkanRenderer::GetInstance();
    VkPipelineLayout layout = vkr.GetPipelineLayout("render").layout;

    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    if (!cameraList.empty())
        this->display = cameraList[0]->colorTexDescSet;

    std::vector<VkImageMemoryBarrier> uiBarriers;
    for (std::shared_ptr<VulkanUI> ui: uiList)
    {
        ZoneScopedN("ExecuteCommand#uiList");
        TracyVkZone(tracyVkCtx, commandBuffer, "ExecuteCommand#uiList");

        barrier.image = ui->colorImage->GetImage();
        uiBarriers.push_back(barrier);

        VkClearValue clearValue{{{0/255.0, 0/255.0, 0/255.0, 1.0f}}};
        VkRenderPassBeginInfo vkRenderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        vkRenderPassInfo.renderPass = vkr.vkRenderPass.imgui;
        vkRenderPassInfo.framebuffer = ui->framebuffer;
        vkRenderPassInfo.renderArea.extent = {
            static_cast<unsigned int>(ui->extent.x),
            static_cast<unsigned int>(ui->extent.y)
        };
        vkRenderPassInfo.clearValueCount = 1;
        vkRenderPassInfo.pClearValues = &clearValue;
        vkCmdBeginRenderPass(commandBuffer, &vkRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        ui->RenderUI();
        vkr.pipelineImgui->RenderUI(ui->drawData,
            ui->vertexBuffer, ui->indexBuffer, commandBuffer);

        vkCmdEndRenderPass(commandBuffer);
    }

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr,
        uiBarriers.size(), uiBarriers.data() 
    );

    std::vector<VkImageMemoryBarrier> camBarriers;
    for (std::shared_ptr<VulkanCamera> camera: cameraList)
    {
        ZoneScopedN("ExecuteCommand#cameraList");
        TracyVkZone(tracyVkCtx, commandBuffer, "ExecuteCommand#cameraList");

        barrier.image = camera->colorImage.GetImage();
        camBarriers.push_back(barrier);

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

        { //skybox
            VkPipelineLayout layout = vkr.GetPipelineLayout("skybox").layout;

            vkCmdBindPipeline(commandBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                vkr.GetPipeline("skybox").pipeline);

            vkCmdBindDescriptorSets(
                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                layout, 1, 1, camera->GetDescriptorSet(), 0, nullptr
            );

            vkCmdBindDescriptorSets(
                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                layout, 0, 1, &this->skyboxTex, 0, nullptr
            );

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(commandBuffer, 0, 1,
                &skyboxMesh->GetVertexbuffer().vertexBuffer, &offset);
            vkCmdBindIndexBuffer(commandBuffer,
                skyboxMesh->GetVertexbuffer().indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, 
                skyboxMesh->GetVertexbuffer().GetIndexCount(), 1, 0, 0, 0);
        }

        vkCmdBindPipeline(commandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            vkr.GetPipeline("render").pipeline);


        vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
            layout, 2, 1, camera->GetDescriptorSet(), 0, nullptr
        );
        vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
            layout, 3, 1, &sceneDescSet, 0, nullptr
        );

        for(const auto& m: renderMesh)
        {
            ZoneScopedN("ExecuteCommand#renderMesh");
            TracyVkZone(tracyVkCtx, commandBuffer, "ExecuteCommand#renderMesh");

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


        { // Wireframe rendering
            VkPipelineLayout layout = vkr.GetPipelineLayout("wire").layout;

            vkCmdBindPipeline(commandBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                vkr.GetPipeline("wire").pipeline);

            vkCmdBindDescriptorSets(
                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                layout, 0, 1, camera->GetDescriptorSet(), 0, nullptr
            );

            for (WirePushConst& e: this->wireList)
            {
                ZoneScopedN("ExecuteCommand#wireList");
                TracyVkZone(tracyVkCtx, commandBuffer, "ExecuteCommand#wireList");

                vkCmdPushConstants(commandBuffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                    sizeof(e), &e);
                vkCmdDraw(commandBuffer, 6, 1, 0, 0);
            }
        }

        vkCmdEndRenderPass(commandBuffer);
    }

    renderMesh.clear();
    cameraList.clear();
    wireList.clear();
    uiList.clear();

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr,
        camBarriers.size(), camBarriers.data() 
    );
}

void RenderTechnique::ScanNode(VulkanNode* node, const glm::mat4& transform)
{
    ZoneScopedN("RenderTechnique::ScanNode");

    glm::mat4 globTransform = transform * node->GetTransform();
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

        if (sceneMap->nLight.x != 5)
        {
            uint32_t i = sceneMap->nLight.x++;
            sceneMap->dirLight[i] = light->dirLight;
        }
    }
    else if (node->camera)
    {
        if (node->camera->cameraType == CameraType::CAMERA)
        {
            std::shared_ptr<VulkanCamera> vkCamera =
                std::dynamic_pointer_cast<VulkanCamera>(node->camera);
            vkCamera->SetTransform(globTransform);

            cameraList.push_back(
                std::dynamic_pointer_cast<VulkanCamera>(node->camera));
        }
        else if (node->camera->cameraType == CameraType::VR_DISPLAY)
        {
            std::shared_ptr<VulkanVrDisplay> vkCamera =
                std::dynamic_pointer_cast<VulkanVrDisplay>(node->camera);

            Input* input = Input::GetInstance();

            {   // project matrices of the left eye
                glm::vec4 xrFoV = input->xr_left_eye_fov;
                glm::mat4 proj;
                XrProjectionFov(proj, xrFoV, 0.01f, 100.0f);

                vkCamera->cameras[0]->vpMap->projection = proj;
            }

            {   // project matrices of the right eye
                glm::vec4 xrFoV = input->xr_right_eye_fov;
                glm::mat4 proj;
                XrProjectionFov(proj, xrFoV, 0.01f, 100.0f);

                vkCamera->cameras[1]->vpMap->projection = proj;
            }

            {   // view matrix of the left eye
                glm::vec4 xrQuat = input->xr_left_eye_quat;
                glm::vec3 xrPos = input->xr_left_eye_pos;
                glm::quat quat{};
                quat.x = xrQuat[0];
                quat.y = xrQuat[1];
                quat.z = xrQuat[2];
                quat.w = xrQuat[3];

                glm::mat4 rotation = glm::toMat4(quat);
                glm::mat4 translation = glm::translate(glm::mat4(1.0f), xrPos);

                vkCamera->cameras[0]->SetTransform(
                    globTransform * translation * rotation);
            }

            {   // view matrix of the right eye
                glm::vec4 xrQuat = input->xr_right_eye_quat;
                glm::vec3 xrPos = input->xr_right_eye_pos;
                glm::quat quat{};
                quat.x = xrQuat[0];
                quat.y = xrQuat[1];
                quat.z = xrQuat[2];
                quat.w = xrQuat[3];

                glm::mat4 rotation = glm::toMat4(quat);
                glm::mat4 translation = glm::translate(glm::mat4(1.0f), xrPos);

                vkCamera->cameras[1]->SetTransform(
                    globTransform * translation * rotation);

                // Correct the transform of HMD object
                globTransform = globTransform * translation * rotation;
                *node->transform = globTransform;
            }

            cameraList.push_back(
                std::dynamic_pointer_cast<VulkanCamera>(vkCamera->cameras[0]));
            cameraList.push_back(
                std::dynamic_pointer_cast<VulkanCamera>(vkCamera->cameras[1]));

            xrDisplay[0] = vkCamera->cameras[0]->colorTexDescSet;
            xrDisplay[1] = vkCamera->cameras[1]->colorTexDescSet;
        }
    } 
    else if (!node->wireList.empty())
    {
        for (WirePushConst& e: node->wireList)
        {
            this->wireList.push_back(e);
        }
    }
    // Although ui is not directly rendered 
    // it still has to be placed in a node to be scanned
    // it can be stored in the same node that the quad is also stored.
    if (node->ui)
    {
        uiList.push_back(std::dynamic_pointer_cast<VulkanUI>(node->ui));
    }
    
    for (const auto& n: node->nodeLists)
    {
        ScanNode(static_cast<VulkanNode*>(n.get()), globTransform);
    }
}

void RenderTechnique::Initialize(VulkanDevice* vulkanDevice)
{
    ZoneScopedN("RenderTechnique::Initialize");

    VulkanRenderer& vkr = VulkanRenderer::GetInstance();
    sceneUniform.Initialize(vulkanDevice, sizeof(SceneData));
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
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = VulkanTexture::GetDefaultTexture()->GetDescriptor();

        vkUpdateDescriptorSets(
            vulkanDevice->vkDevice, 1, &descriptorWrite, 0, nullptr);
    }

    this->display = defaultDisplay;

    { //setup default skybox
        skyboxMesh = std::make_shared<VulkanMesh>();
        BuildMeshInfo info{};

        std::string path = "resources/models/gltf/cube.gltf";
        GltfModel cube{};
        cube.LoadModel(path);

        skyboxMesh = cube.meshList[0];

        defaultSkybox = true;

        TextureCubeBuildInfo texInfo{};
        textureCube = std::dynamic_pointer_cast<VulkanTextureCube>(
            VulkanTextureCube::BuildTexture(texInfo));
        

        VulkanPipelineLayout& layout = vkr.GetPipelineLayout("skybox");
        layout.AllocateDescriptorSet("textureCube", vkr.FRAME_IN_FLIGHT, &skyboxTex);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = this->skyboxTex;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = VulkanTextureCube::GetDefaultTexture()->GetDescriptor();

        vkUpdateDescriptorSets(
            vulkanDevice->vkDevice, 1, &descriptorWrite, 0, nullptr);
    }
}

void RenderTechnique::DrawCamera(VkCommandBuffer vkCommandBuffer)
{
    ZoneScopedN("RenderTechnique::DrawCamera");
}

} // namespace renderer
