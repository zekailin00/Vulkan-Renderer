#pragma once

#include "application.h"

class Viewport
{
public:
    void SetScene(Scene* scene)
    {
        this->scene = scene;
    }

    void Draw()
    {
        if (scene == nullptr)
            return;
        // if (ImGui::BeginTabBar("CameraList", ImGuiTabBarFlags_None))
        // {
        //     ImGui::EndTabBar();
        // }

        ImGui::Begin("Viewport", nullptr);

        std::vector<Entity*> entityList;
        scene->GetEntitiesWithComponent(
            Component::Type::Camera,
            entityList
        );

        for (Entity* e: entityList)
        {
            renderer::CameraComponent* comp = (renderer::CameraComponent*)
                e->GetComponent(Component::Type::Camera);

            ImGui::Image(
                (ImTextureID)(*(comp->camera->GetTextureDescriptorSet())),
                {(float)comp->camera->GetCamProperties().Extent.x,
                 (float)comp->camera->GetCamProperties().Extent.y}
            );
        }



        ImGui::End();
    }
private:
    Scene* scene;
};