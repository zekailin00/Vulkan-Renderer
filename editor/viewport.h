#pragma once

#include "application.h"

class Viewport
{
public:
    void SetScene(Scene* scene)
    {
        this->scene = scene;
    }

    void Draw(ImGuiID dockID)
    {
        if (scene == nullptr)
            return;

        std::vector<Entity*> entityList;
        scene->GetEntitiesWithComponent(
            Component::Type::Camera,
            entityList
        );

        ImGuiID nextWindowDockID = dockID;

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
private:
    Scene* scene = nullptr;
};