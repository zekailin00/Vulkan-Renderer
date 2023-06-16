#include "viewport.h"

#include "camera_component.h"
#include "event_queue.h"
#include "events.h"
#include "vulkan_renderer.h"
#include "validation.h"

Viewport::Viewport()
{
    {
        renderer::TextureBuildInfo info{};
        info.imagePath = "resources/cover_image1.png";
        launcherTexture = std::dynamic_pointer_cast<renderer::VulkanTexture>(
            renderer::VulkanTexture::BuildTexture(&info));

        renderer::VulkanRenderer& vkr = renderer::VulkanRenderer::GetInstance();

        VulkanPipelineLayout& layout = vkr.GetPipelineLayout("display");
        layout.AllocateDescriptorSet("texture", vkr.FRAME_IN_FLIGHT, &launcherDescSet);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = launcherDescSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = launcherTexture->GetDescriptor();

        vkUpdateDescriptorSets(
            vkr.vulkanDevice.vkDevice, 1, &descriptorWrite, 0, nullptr);
    }

    subscriberHandle = EventQueue::GetInstance()->Subscribe(EventQueue::Editor,
        [this](Event* event){
            if (event->type == Event::Type::SceneOpen)
            {
                EventSceneOpen* e = reinterpret_cast<EventSceneOpen*>(event);
                this->scene = reinterpret_cast<Scene*>(e->scene);
            }
            else if (event->type == Event::Type::CloseProject)
            {
                this->scene = nullptr;
            }
            else if (event->type == Event::Type::SceneOpen)
            {
                EventSceneOpen* e = reinterpret_cast<EventSceneOpen*>(event);
                this->scene = reinterpret_cast<Scene*>(e->scene);
            }
        });
}

Viewport::~Viewport()
{
    EventQueue::GetInstance()->Unsubscribe(subscriberHandle);
}

void Viewport::DrawNoCamera(ImGuiID& nextWindowDockID)
{
     ImGui::SetNextWindowDockID(nextWindowDockID); 
        ImGui::Begin("Camera##none", nullptr, ImGuiWindowFlags_NoMove);

        ImVec2 contentExtent = ImGui::GetContentRegionAvail();
        glm::vec2 imageExtent = launcherTexture->GetExtent();

        float heightRatio = imageExtent.y / contentExtent.y;
        float widthRatio = imageExtent.x / contentExtent.x;

        if (heightRatio > widthRatio)
        {
            imageExtent.x /= heightRatio;
            imageExtent.y /= heightRatio;
        }
        else
        {
            imageExtent.x /= widthRatio;
            imageExtent.y /= widthRatio;
        }
        
        ImGui::SetCursorPos(ImVec2(
            (contentExtent.x - imageExtent.x) * 0.5f + ImGui::GetCursorPos().x, 
            (contentExtent.y - imageExtent.y) * 0.5f + ImGui::GetCursorPos().y));
        ImGui::Image(
            launcherDescSet, {imageExtent.x, imageExtent.y}
        );

        nextWindowDockID = ImGui::GetWindowDockID();

        ImGui::End();
}

void Viewport::Draw(ImGuiID dockID)
{
    ImGuiID nextWindowDockID = dockID;

    if (!scene)
    {
        DrawNoCamera(nextWindowDockID);
        return;
    }

    std::vector<Entity*> entityList;
    scene->GetEntitiesWithComponent(
        Component::Type::Camera,
        entityList
    );

    if (entityList.empty())
    {
        DrawNoCamera(nextWindowDockID);
        return;
    }

    for (Entity* e: entityList)
    {
        ImGui::SetNextWindowDockID(nextWindowDockID); 
        ImGui::Begin(e->GetName().c_str(), nullptr, ImGuiWindowFlags_NoMove);

        renderer::CameraComponent* comp = (renderer::CameraComponent*)
            e->GetComponent(Component::Type::Camera);

        ImVec2 contentExtent = ImGui::GetContentRegionAvail();
        glm::vec2 imageExtent = comp->camera->GetCamProperties().Extent;

        float heightRatio = imageExtent.y / contentExtent.y;
        float widthRatio = imageExtent.x / contentExtent.x;

        if (heightRatio > widthRatio)
        {
            imageExtent.x /= heightRatio;
            imageExtent.y /= heightRatio;
        }
        else
        {
            imageExtent.x /= widthRatio;
            imageExtent.y /= widthRatio;
        }
        
        ImGui::SetCursorPos(ImVec2(
            (contentExtent.x - imageExtent.x) * 0.5f + ImGui::GetCursorPos().x, 
            (contentExtent.y - imageExtent.y) * 0.5f + ImGui::GetCursorPos().y));
        ImGui::Image(
            *(comp->camera->GetTextureDescriptorSet()),
            {imageExtent.x, imageExtent.y}, {0, 1}, {1, 0}
        );

        nextWindowDockID = ImGui::GetWindowDockID();

        ImGui::End();
    }
}
