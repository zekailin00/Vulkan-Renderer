#include "viewport.h"

#include "camera_component.h"
#include "openxr_components.h"

#include "event_queue.h"
#include "events.h"
#include "vulkan_renderer.h"
#include "validation.h"

typedef std::pair<std::string, std::shared_ptr<renderer::VulkanCamera>> ViewportTarget;

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
            else if (event->type == Event::Type::CloseScene)
            {
                this->scene = nullptr;
            }
            else if (event->type == Event::Type::SceneSelected)
            {
                EventSceneSelected* e = reinterpret_cast<EventSceneSelected*>(event);
                this->scene = reinterpret_cast<Scene*>(e->scene);
            }
            else if (event->type == Event::Type::SimStart)
            {
                this->scene = nullptr;
            }
            else if (event->type == Event::Type::SimStartVR)
            {
                this->scene = nullptr;
            }
            else if (event->type == Event::Type::SimStop)
            {
                this->scene = nullptr;
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

        ImGui::End();
}

void Viewport::Draw(ImGuiID dockID)
{

    if (!scene)
    {
        DrawNoCamera(dockID);
        return;
    }
    
    std::vector<ViewportTarget> entityList;

    scene->GetRootEntity()->ScanEntities(
        [&entityList](Entity* e){
            if (e->HasComponent(Component::Type::Camera))
            {
                entityList.push_back({
                    e->GetName(),
                    ((renderer::CameraComponent*)
                    e->GetComponent(Component::Type::Camera))->camera
                });
            }
            if (e->HasComponent(Component::Type::VrDisplay))
            {
                // If entity has a VR display component,
                // only show th left camera.
                entityList.push_back({
                    e->GetName() + " (VR)",
                    ((renderer::VrDisplayComponent*)
                    e->GetComponent(Component::Type::VrDisplay))
                        ->vrDisplay->GetLeftCamera()
                });
            }
        }
    );

    if (entityList.empty())
    {
        DrawNoCamera(dockID);
        return;
    }

    for (int i = 0; i < entityList.size(); i++)
    {
        const ViewportTarget& t = entityList[i];

        // Window is docked to main window when first created.
        if (cameraDocking.find(t.first.c_str()) == cameraDocking.cend())
        {
            ImGui::SetNextWindowDockID(dockID);
            cameraDocking.insert(t.first);
        }

        ImGui::Begin(t.first.c_str(), nullptr);

        ImVec2 contentExtent = ImGui::GetContentRegionAvail();
        glm::vec2 imageExtent = t.second->GetCamProperties().Extent;

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
            *(t.second->GetTextureDescriptorSet()),
            {imageExtent.x, imageExtent.y}, {0, 1}, {1, 0}
        );

        ImGui::End();
    }
}
