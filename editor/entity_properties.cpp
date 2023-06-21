#include "entity_properties.h"

#include "light_component.h"
#include "camera_component.h"
#include "openxr_components.h"

#include "asset_manager.h"
#include "validation.h"


EntityProperties::EntityProperties()
{
    subscriberHandle = EventQueue::GetInstance()->Subscribe(EventQueue::Editor,
        [this](Event* event){
            if (event->type == Event::Type::EntitySelected)
            {
                EventEntitySelected* e = dynamic_cast<EventEntitySelected*>(event);
                this->selectedEntity = e->entity;
            }
            else if (event->type == Event::Type::DeleteEntity)
            {
                EventDeleteEntity* e = dynamic_cast<EventDeleteEntity*>(event);
                if (e->entity == this->selectedEntity)
                {
                    this->selectedEntity = nullptr;
                }
            }
            else if (event->type == Event::Type::ProjectOpen)
            {
                EventProjectOpen* e = dynamic_cast<EventProjectOpen*>(event);
                this->assetManager = reinterpret_cast<AssetManager*>(e->assetManager);
            }
            else if (event->type == Event::Type::CloseProject)
            {
                this->selectedEntity = nullptr;
                this->availableMaterials.clear();
                this->availableMaterialCached = false;
                this->availableMeshes.clear();
                this->availableMeshCached = false;
                this->assetManager = nullptr;
            }
            else if (event->type == Event::Type::SceneOpen)
            {
                this->selectedEntity = nullptr;
            }
            else if (event->type == Event::Type::CloseScene)
            {
                this->selectedEntity = nullptr;
            }
            else if (event->type == Event::Type::WorkspaceChanged)
            {
                this->availableMaterials.clear();
                this->availableMaterialCached = false;
                this->availableMeshes.clear();
                this->availableMeshCached = false;
            }
            else if (event->type == Event::Type::SimStart)
            {
                this->selectedEntity = nullptr;
            }
            else if (event->type == Event::Type::SimStartVR)
            {
                this->selectedEntity = nullptr;
            }
            else if (event->type == Event::Type::SimStop)
            {
                this->selectedEntity = nullptr;
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
    ImGui::SameLine(ImGui::GetContentRegionAvail().x-30);
    AddComponent();

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Entity Properties"))
    {
        ImGui::SeparatorText("Name");
        {
            char nameBuffer[128];
            ImGui::PushItemWidth(-0.1);
            strcpy_s(nameBuffer, selectedEntity->GetName().c_str());
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

        ImGui::SeparatorText("Translation, Rotation, Scale");
        {
            glm::vec3 t = selectedEntity->GetLocalTranslation();

            // Workaround for rotation conversion issue in glm library.
            static bool uiControlling = false;
            static glm::vec3 r;
            static int frameCount = -1;
            if (!uiControlling && frameCount-- < 0)
            {
                // Euler angles, pitch as x, yaw as y, roll as z.
                r = selectedEntity->GetLocalRotation();
                frameCount = -1;
            }

            glm::vec3 s = selectedEntity->GetLocalScale();

            ImGui::DragFloat3("Translation", &t[0], 0.01f, -FLT_MAX, FLT_MAX);

            if (ImGui::DragFloat3("Rotation", &r[0], 0.01f, -FLT_MAX, FLT_MAX))
            {
                uiControlling = true;
                frameCount = 100;
            }
            else
            {
                uiControlling = false;
            }

            ImGui::DragFloat3("Scale", &s[0], 0.001f, 0.001, FLT_MAX,
                "%.3f", ImGuiSliderFlags_AlwaysClamp);

            selectedEntity->SetLocalTransform(t, r, s);
        }

        ImGui::Separator();
    }

    if (selectedEntity->HasComponent(Component::Type::Light))
    {
        ShowLightComponent();
    }

    if (selectedEntity->HasComponent(Component::Type::Camera))
    {
        ShowCameraComponent();
    }

    if (selectedEntity->HasComponent(Component::Type::VrDisplay))
    {
        ShowVrDisplayComponent();
    }

    if (selectedEntity->HasComponent(Component::Type::Mesh))
    {
        ShowMeshComponent();
    }

    if (selectedEntity->HasComponent(Component::Type::Wireframe))
    {
        ShowWireframeComponent();
    }
}

void EntityProperties::ShowLightComponent()
{
    if (ImGui::CollapsingHeader("Light Component"))
    {

        RemoveComponent(Component::Type::Light);

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
            ImGui::ColorEdit3("Color##1", color,
            ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_DisplayRGB |
            ImGuiColorEditFlags_Float | ImGuiColorEditFlags_InputRGB);
        }

        ImGui::Separator();
    }
}

void EntityProperties::ShowCameraComponent()
{
    if (ImGui::CollapsingHeader("Camera Component"))
    {
        RemoveComponent(Component::Type::Camera);

        renderer::CameraComponent* component =
            dynamic_cast<renderer::CameraComponent*>(
                selectedEntity->GetComponent(Component::Type::Camera));

        // FIXME: component has a copy of cam prop, cam also has one
        // it better to just keep one copy.
        // writes to component or writes to camera resource??
        const renderer::CameraProperties& prop =
            component->camera->GetCamProperties();

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.4);

        ImGui::SeparatorText("Resolution");
        {
            ImGui::BeginDisabled();
            int extent[2] = {(int)prop.Extent.x, (int)prop.Extent.y};
            ImGui::DragInt2("Resolution (constant)", extent,
                0, 360, 4096, "%d");
            ImGui::EndDisabled();
        }

        ImGui::SeparatorText("Projection");
        {
            float aspectRatio = prop.Extent.x / (float)prop.Extent.y;
            float fovy = prop.Fov;
            float zNear = prop.ZNear;
            float zFar = prop.ZFar;

            ImGui::BeginDisabled();
            ImGui::DragFloat("Aspect Ratio (x/y)", &aspectRatio,
                0.0f, 0.001f, 1000, "%.3f");
            ImGui::EndDisabled();

            ImGui::DragFloat("Field of View (y axis in degree)", &fovy,
                0.1f, 10.0f, 170.0f, "%.1f");
            ImGui::DragFloat("Near Plane (meter)", &zNear,
                0.01f, 0.001f, FLT_MAX, "%.3f");
            ImGui::DragFloat("Far Plane (meter)", &zFar,
                1.0f, 20.0f, FLT_MAX, "%.1f");

            // FIXME: component has a copy of cam prop, cam also has one
            // it better to just keep one copy. 
            component->camera->SetProjection(
                prop.Extent.x / (float)prop.Extent.y, fovy, zNear, zFar);
        }

        if (ImGui::TreeNode("Rebuild Camera"))
        {
            ImGui::SeparatorText("Camera Properties");
            static renderer::CameraProperties prop{};
            static float extent[2];

            prop.UseFrameExtent = false;
            extent[0] = static_cast<float>(prop.Extent.x);
            extent[1] = static_cast<float>(prop.Extent.y);

            ImGui::DragFloat2("Resolution", extent,
                10.0f, 360, 4100, "%.1f", ImGuiSliderFlags_AlwaysClamp);

            prop.Extent.x = static_cast<unsigned int>(extent[0]);
            prop.Extent.y = static_cast<unsigned int>(extent[1]);

            ImGui::DragFloat("Field of View", &prop.Fov,
                0.1f, 10.0f, 170.0f, "%.1f");
            ImGui::DragFloat("Near Plane", &prop.ZNear,
                0.01f, 0.001f, FLT_MAX, "%.3f");
            ImGui::DragFloat("Far Plane", &prop.ZFar,
                1.0f, 20.0f, FLT_MAX, "%.1f");


            // FIXME: camera rebuild has LAYOUT_UNDEFINED bug
            if (ImGui::SmallButton("Rebuild"))
            {
                component->camera->RebuildCamera(prop);
            }

            ImGui::SameLine();
            if (ImGui::SmallButton("Reset"))
            {
                renderer::CameraProperties defaultProp{};
                defaultProp.UseFrameExtent = false;
                prop = defaultProp;
                component->camera->RebuildCamera(defaultProp);
            }

            ImGui::TreePop();
        }

        ImGui::PopItemWidth();

        ImGui::Separator();
    }
}

void EntityProperties::ShowVrDisplayComponent()
{
    if (ImGui::CollapsingHeader("VR Display Component"))
    {
        RemoveComponent(Component::Type::VrDisplay);

        // renderer::VrDisplayComponent* component =
        //     dynamic_cast<renderer::VrDisplayComponent*>(
        //         selectedEntity->GetComponent(Component::Type::VrDisplay));

        ImGui::SeparatorText("VR Display");
        {
            ImGui::Text("Nothing to show :(");
        }

        ImGui::Separator();
    }
}

void EntityProperties::ShowMeshComponent()
{
    if (ImGui::CollapsingHeader("Mesh Component"))
    {

        RemoveComponent(Component::Type::Mesh);

        renderer::MeshComponent* component =
            dynamic_cast<renderer::MeshComponent*>(
                selectedEntity->GetComponent(Component::Type::Mesh));

        ImGui::SeparatorText("Mesh");
        {

            if (!availableMeshCached)
            {
                assetManager->GetAvailableMeshes(availableMeshes);
                availableMeshCached = true;
            }

            std::string currentMeshStr = component->mesh?
                component->mesh->GetResourcePath(): "None";
            const char* currentMeshPath = currentMeshStr.c_str();
            if (ImGui::BeginCombo("Mesh Path", currentMeshPath))
            {
                for (int n = 0; n < availableMeshes.size(); n++)
                {
                    const bool isSelected =
                        strcmp(availableMeshes[n], currentMeshPath) == 0;
                    if (ImGui::Selectable(availableMeshes[n], isSelected))
                    {
                        component->mesh =
                            std::dynamic_pointer_cast<renderer::VulkanMesh> 
                            (assetManager->GetMesh(availableMeshes[n]));
                    }

                    // Set the initial focus to the selected mesh
                    // when opening the combo
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }
        
        ImGui::SeparatorText("Material");
        if (component->mesh)
        {

            if (!availableMaterialCached)
            {
                assetManager->GetAvailableMaterials(availableMaterials);
                availableMaterialCached = true;
            }

            std::shared_ptr<renderer::VulkanMaterial> material =
                component->mesh->GetVulkanMaterial();

            //FIXME: path "/" "\\" mismatch,
            // all paths when recorded to resources
            // need to be converted to unix path format.
            std::string currentMaterialStr =
                material->GetProperties()->resourcePath;
            const char* currentMaterialPath = currentMaterialStr.c_str();
            if (ImGui::BeginCombo("Material Path", currentMaterialPath))
            {
                for (int n = 0; n < availableMaterials.size(); n++)
                {
                    const bool isSelected =
                        strcmp(availableMaterials[n], currentMaterialPath) == 0;
                    if (ImGui::Selectable(availableMaterials[n], isSelected))
                    {
                        component->mesh->AddMaterial(
                            std::dynamic_pointer_cast<renderer::VulkanMaterial> 
                            (assetManager->GetMaterial(availableMaterials[n])));
                    }

                    // Set the initial focus to the selected mesh
                    // when opening the combo
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            bool openMatEditor = ImGui::SmallButton("Open in Editor");
            if (openMatEditor)
            {
                PublishMaterialSelectedEvent(material.get());
            }
            
        }
        ImGui::Separator();
    }
}

void EntityProperties::ShowWireframeComponent()
{
    if (ImGui::CollapsingHeader("Wireframe Component"))
    {
        RemoveComponent(Component::Type::Wireframe);

        // renderer::WireframeComponent* component =
        //     dynamic_cast<renderer::WireframeComponent*>(
        //         selectedEntity->GetComponent(Component::Type::Wireframe));

        ImGui::SeparatorText("Wireframe");
        ImGui::Text("TODO:");
        ImGui::Separator();
    }
}

void EntityProperties::AddComponent()
{
    std::vector<std::string> components
    {
        "Light Component",
        "Camera Component",
        "VR Display Component",
        "Mesh Component",
    };

    std::string selectedComponent;
    if (ImGui::SmallButton("New"))
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

    if (selectedComponent == "Light Component" &&
        !selectedEntity->HasComponent(Component::Type::Light))
    {
        selectedEntity->AddComponent(Component::Type::Light);
    }
    else if (selectedComponent == "Camera Component" &&
        !selectedEntity->HasComponent(Component::Type::Camera))
    {
        selectedEntity->AddComponent(Component::Type::Camera);
    }
    else if (selectedComponent == "VR Display Component" &&
        !selectedEntity->HasComponent(Component::Type::VrDisplay))
    {
        selectedEntity->AddComponent(Component::Type::VrDisplay);
    }
    else if (selectedComponent == "Mesh Component" &&
        !selectedEntity->HasComponent(Component::Type::Mesh))
    {
        selectedEntity->AddComponent(Component::Type::Mesh);
    }
}

void EntityProperties::RemoveComponent(Component::Type type)
{
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Button("Remove Component"))
        {
            EventDeleteComponent* event = new EventDeleteComponent();
            event->entity = selectedEntity;
            event->componentType = type;
            EventQueue::GetInstance()->Publish(
                EventQueue::Editor, event);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Right-click to open popup");
}

void EntityProperties::PublishMaterialSelectedEvent(
    renderer::VulkanMaterial* mat)
{
    ASSERT(mat != nullptr);

    EventMaterialSelected* event = new EventMaterialSelected();
    event->materialPtr = mat;

    EventQueue::GetInstance()->Publish(EventQueue::Editor,event);
}