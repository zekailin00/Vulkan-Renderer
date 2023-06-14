#include "entity_properties.h"

#include "light_component.h"
#include "camera_component.h"


EntityProperties::EntityProperties()
{
    subscriberHandle = EventQueue::GetInstance()->Subscribe(EventQueue::Editor,
        [this](Event* event){
            if (event->type == Event::Type::EntitySelected)
            {
                EventEntitySelected* e = dynamic_cast<EventEntitySelected*>(event);
                this->selectedEntity = e->entity;
            }
            if (event->type == Event::Type::DeleteEntity)
            {
                EventEntitySelected* e = dynamic_cast<EventEntitySelected*>(event);
                if (e->entity == this->selectedEntity)
                {
                    this->selectedEntity = nullptr;
                }
            }
        });
}

EntityProperties::~EntityProperties()
{
    EventQueue::GetInstance()->Unsubscribe(subscriberHandle);
}


void EntityProperties::Draw()
{
    ImGui::Begin("Entity Properties", nullptr);
    ImGui::Text("Properties of the selected entity");
    ImGui::Separator();

    ShowEntityProperties();
    
    ImGui::End();
}

void EntityProperties::ShowEntityProperties()
{
    if(!selectedEntity)
    {
        ImGui::Text("No entity is selected.");
        return;
    }
        
    ImGui::Text("%s", selectedEntity->GetName().c_str());
    ImGui::SameLine(ImGui::GetWindowWidth()-40);
    AddComponent();

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Entity Properties"))
    {
        ImGui::SeparatorText("Name");
        {
            char nameBuffer[128];
            ImGui::PushItemWidth(-0.1);
            strcpy(nameBuffer, selectedEntity->GetName().c_str());
            if (ImGui::InputText(
                "##identifier", nameBuffer, IM_ARRAYSIZE(nameBuffer),
                ImGuiInputTextFlags_EnterReturnsTrue))
            {
                selectedEntity->SetName(std::string(nameBuffer));
            }
            ImGui::PopItemWidth();
        }

        ImGui::SeparatorText("Global Transform");
        {
            glm::mat4 globalTransform = selectedEntity->GetGlobalTransform();
            globalTransform = glm::transpose(globalTransform);
            float* row0 = &globalTransform[0][0];
            float* row1 = &globalTransform[1][0];
            float* row2 = &globalTransform[2][0];
            float* row3 = &globalTransform[3][0];

            ImGui::PushItemWidth(-0.1);
            ImGui::DragFloat4("##row0g", row0, 0.0f, -FLT_MAX, FLT_MAX,
                "%.3f", ImGuiSliderFlags_NoInput);
            ImGui::DragFloat4("##row1g", row1, 0.0f, -FLT_MAX, FLT_MAX,
                "%.3f", ImGuiSliderFlags_NoInput);
            ImGui::DragFloat4("##row2g", row2, 0.0f, -FLT_MAX, FLT_MAX,
                "%.3f", ImGuiSliderFlags_NoInput);
            ImGui::DragFloat4("##row3g", row3, 0.0f, -FLT_MAX, FLT_MAX,
                "%.3f", ImGuiSliderFlags_NoInput);
            ImGui::PopItemWidth();
        }

        ImGui::SeparatorText("Local Transform");
        {
            glm::mat4 localTransform = selectedEntity->GetLocalTransform();
            localTransform = glm::transpose(localTransform);
            float* row0 = &localTransform[0][0];
            float* row1 = &localTransform[1][0];
            float* row2 = &localTransform[2][0];
            float* row3 = &localTransform[3][0];

            ImGui::PushItemWidth(-0.1);
            ImGui::DragFloat4("##row0l", row0, 0.01f,
                -FLT_MAX, FLT_MAX, "%.3f");
            ImGui::DragFloat4("##row1l", row1, 0.01f,
                -FLT_MAX, FLT_MAX, "%.3f");
            ImGui::DragFloat4("##row2l", row2, 0.01f,
                -FLT_MAX, FLT_MAX, "%.3f");
            ImGui::DragFloat4("##row3l", row3, 0.01f,
                -FLT_MAX, FLT_MAX, "%.3f");
            ImGui::PopItemWidth();

            selectedEntity->SetLocalTransform(glm::transpose(localTransform));
        }

        ImGui::Separator();
    }

    if (selectedEntity->HasComponent(Component::Type::Light))
    {
        if (ImGui::CollapsingHeader("Light Component"))
        {
            renderer::LightComponent* component =
                dynamic_cast<renderer::LightComponent*>(
                    selectedEntity->GetComponent(Component::Type::Light));
            
            ImGui::SeparatorText("Type");
            {
                ImGui::Text("Directional Light");
            }
            ImGui::SeparatorText("Color");
            {
                float* color = &component->properties.color[0];
                ImGui::ColorEdit3("MyColor##1", color,
                ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_Float | ImGuiColorEditFlags_InputRGB);
            }

            ImGui::Separator();
        }
    }

    if (selectedEntity->HasComponent(Component::Type::Camera))
    {
        if (ImGui::CollapsingHeader("Camera Component"))
        {
            renderer::CameraComponent* component =
                dynamic_cast<renderer::CameraComponent*>(
                    selectedEntity->GetComponent(Component::Type::Camera));

            const renderer::CameraProperties& prop =
                component->camera->GetCamProperties();

            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.4);

            ImGui::SeparatorText("Resolution");
            {
                int extent[2] = {prop.Extent.x, prop.Extent.y};
                ImGui::DragInt2("Resolution (constant)", extent,
                    0, 360, 4096, "%d", ImGuiSliderFlags_NoInput);
            }

            ImGui::SeparatorText("Projection");
            {
                float aspectRatio = prop.Extent.x / (float)prop.Extent.y;
                float fovy = prop.Fov;
                float zNear = prop.ZNear;
                float zFar = prop.ZFar;

                ImGui::DragFloat("Aspect Ratio (x/y)", &aspectRatio,
                    0.0f, 0.001f, 1000, "%.3f", ImGuiSliderFlags_NoInput);
                ImGui::DragFloat("Field of View (y axis in degree)", &fovy,
                    0.01f, 10.0f, 300.0f, "%.2f");
                ImGui::DragFloat("Near Plane (meter)", &zNear,
                    0.01f, 0.01f, FLT_MAX, "%.2f");
                ImGui::DragFloat("Far Plane (meter)", &zFar,
                    0.01f, 0.01f, FLT_MAX, "%.2f");

                component->camera->SetProjection(
                    prop.Extent.x / (float)prop.Extent.y, fovy, zNear, zFar);
            }

            ImGui::PopItemWidth();

            static bool rebuildCamera = false;
            if (ImGui::Button("Rebuilt Camera"))
            {
                rebuildCamera = true;
            }

            ImGui::Separator();
        }
    }
}


void EntityProperties::AddComponent()
{
    std::vector<std::string> components
    {
        "Mesh Component",
        "Camera Component",
        "Physics Component",
        "Javascript Component"
    };

    std::string selectedComponent;
    if (ImGui::Button("New"))
        ImGui::OpenPopup("NewComponents");
    if (ImGui::BeginPopup("NewComponents"))
    {
        ImGui::Text("Components");
        ImGui::Separator();
        for (int i = 0; i < components.size(); i++)
            if (ImGui::Selectable(components[i].c_str()))
                selectedComponent = components[i];
        ImGui::EndPopup();
    }

    // //TODO: prevent adding one that exists already
    // if (selectedComponent == "Mesh Component")
    // {
    //     MeshComponent& mesh = context->ShowProperties.AddComponent<MeshComponent>();
    //     mesh.LoadMesh("resources/models/cube/cube.obj");
    //     mesh.LoadTexture("resources/textures/defaultTexture.png");
    //     mesh.SetTextureEnabled(true);
    // }
    // else if (selectedComponent == "Camera Component")
    // {
    //     CameraComponent& camera = context->ShowProperties.AddComponent<CameraComponent>();
    // }
    // else if (selectedComponent == "Javascript Component")
    // {
    //     JavascriptComponent& script = 
    //         context->ShowProperties.AddComponent<JavascriptComponent>();
    // }
}