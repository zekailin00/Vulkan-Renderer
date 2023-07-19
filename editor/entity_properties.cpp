#include "entity_properties.h"

#include "light_component.h"
#include "camera_component.h"
#include "openxr_components.h"
#include "script_component.h"

#include "asset_manager.h"
#include "validation.h"
#include "logger.h"
#include "math_library.h"

#include <map>


static void DrawPhysicsShapeCommon(physics::CollisionShape* shape);

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
                this->availableScripts.clear();
                this->availableScriptCached = false;
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
                this->availableScripts.clear();
                this->availableScriptCached = false;
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
            std::string entityName = selectedEntity->GetName();
            strncpy(nameBuffer, entityName.c_str(), entityName.size());
            nameBuffer[entityName.size()] = '\0';
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

            bool edited = false;

            ImGui::PushItemWidth(-0.1);
            edited = ImGui::DragFloat4("##row0l",
                row0, 0.01f, -FLT_MAX, FLT_MAX, "%.3f") || edited;
            edited = ImGui::DragFloat4("##row1l",
                row1, 0.01f, -FLT_MAX, FLT_MAX, "%.3f") || edited;
            edited = ImGui::DragFloat4("##row2l",
                row2, 0.01f, -FLT_MAX, FLT_MAX, "%.3f") || edited;
            edited = ImGui::DragFloat4("##row3l",
                row3, 0.01f, -FLT_MAX, FLT_MAX, "%.3f") || edited;
            ImGui::PopItemWidth();

            if (edited)
            {
                selectedEntity->SetLocalTransform(glm::transpose(localTransform));
            }
        }

        ImGui::SeparatorText("Translation, Rotation, Scale");
        {
            glm::vec3 t = selectedEntity->GetLocalTranslation();
            glm::vec3 r = selectedEntity->GetLocalRotation();
            glm::vec3 s = selectedEntity->GetLocalScale();

            bool edited = false;

            if (ImGui::DragFloat3("Translation", &t[0],
                0.01f, -FLT_MAX, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
            {
                glm::mat4 transform = selectedEntity->GetLocalTransform();
                transform[3][0] = t[0];
                transform[3][1] = t[1];
                transform[3][2] = t[2];

                selectedEntity->SetLocalTransform(transform);
            }

            glm::vec3 rCached = r;
            if (ImGui::DragFloat3("Rotation", &r[0], 0.01f,
                -FLT_MAX, FLT_MAX, "%.3f",
                ImGuiSliderFlags_AlwaysClamp))
            {
                if (rCached[0] != r[0])
                {
                    if (glm::abs(rCached[0] - r[0]) > 0.1f)
                    {
                        selectedEntity->SetLocalRotation(r);
                    }
                    else
                    {
                        glm::mat4 transform = selectedEntity->GetLocalTransform();
                        math::RotateAroundBasis0(transform, r[0] - rCached[0]);
                        selectedEntity->SetLocalTransform(transform);
                    }
                }
                else if (rCached[1] != r[1])
                {
                    if (glm::abs(rCached[1] - r[1]) > 0.1f)
                    {
                        selectedEntity->SetLocalRotation(r);
                    }
                    else
                    {
                        glm::mat4 transform = selectedEntity->GetLocalTransform();
                        math::RotateAroundBasis1(transform, r[1] - rCached[1]);
                        selectedEntity->SetLocalTransform(transform);
                    }
                }
                else if (rCached[2] != r[2])
                {
                    if (glm::abs(rCached[2] - r[2]) > 0.1f)
                    {
                        selectedEntity->SetLocalRotation(r);
                    }
                    else
                    {
                        glm::mat4 transform = selectedEntity->GetLocalTransform();
                        math::RotateAroundBasis2(transform, r[2] - rCached[2]);
                        selectedEntity->SetLocalTransform(transform);
                    }
                }
            }


            if (ImGui::DragFloat3("Scale", &s[0],
                0.01f, 0.01f, FLT_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp))
            {
                if (s[0] != 0 && s[1] != 0 && s[2] != 0)
                {
                    glm::mat4 transform = selectedEntity->GetLocalTransform();
                    transform[0] = glm::normalize(transform[0]) * s[0];
                    transform[1] = glm::normalize(transform[1]) * s[1];
                    transform[2] = glm::normalize(transform[2]) * s[2];

                    selectedEntity->SetLocalTransform(transform);
                }
            }
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

    if (selectedEntity->HasComponent(Component::Type::Line))
    {
        ShowWireframeComponent();
    }

    if (selectedEntity->HasComponent(Component::Type::Script))
    {
        ShowScriptComponent();
    }

    if (selectedEntity->HasComponent(Component::Type::DynamicBody))
    {
        ShowDynamicBodyComponent();
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
    if (ImGui::CollapsingHeader("Line Component"))
    {
        RemoveComponent(Component::Type::Line);

        renderer::LineComponent* component =
            dynamic_cast<renderer::LineComponent*>(
                selectedEntity->GetComponent(Component::Type::Line));

        ImGui::SeparatorText("Line Properties");
        {
            renderer::LineRenderer::LineProperties* prop =
                component->lineRenderer->GetLineProperties();

            glm::vec3 color = prop->color;
            if (ImGui::ColorEdit3("Color##1", &color[0],
                ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_Float | ImGuiColorEditFlags_InputRGB))
            {
                prop->color = color;
            }

            int pixel = prop->width;
            if (ImGui::DragInt("Width in pixel", &pixel, 1.0f, 1, 2049, "%d",
                ImGuiSliderFlags_AlwaysClamp))
            {
                prop->width = pixel;
            }

            bool useTransform = prop->useGlobalTransform;
            if (ImGui::Checkbox("Use Global Transform", &useTransform))
            {
                prop->useGlobalTransform = useTransform;
            }

        }

        ImGui::SeparatorText("Line Data");
        {
            int removeIndex = -1; 
            int lineCount = component->lineRenderer->GetInstanceCount();
            renderer::VulkanBuffer<renderer::LineData>* lineData =
                component->lineRenderer->GetLineData();

            ImGuiTableFlags flags =
                ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchProp |
                ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_Hideable |
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;

            int freezeCols = 1;
            int freezeRows = 1;

            ImVec2 outer_size = ImVec2(0.0f,
                ImGui::GetTextLineHeightWithSpacing() * 8);
            if (ImGui::BeginTable("table_scrollx", 4, flags, outer_size))
            {
                ImGui::TableSetupScrollFreeze(freezeCols, freezeRows);
                ImGui::TableSetupColumn("Pts", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Begin");
                ImGui::TableSetupColumn("End");
                ImGui::TableSetupColumn("...");

                ImGui::TableHeadersRow();

                for (int row = 0; row < lineCount; row++)
                {
                    ImGui::TableNextRow();
                    for (int column = 0; column < 4; column++)
                    {
                        if (!ImGui::TableSetColumnIndex(column) && column > 0)
                            continue;

                        renderer::LineData& data = (*lineData)[row];

                        if (column == 0)
                        {
                            ImGui::Text("%d", row);
                        }
                        else if (column == 1)
                        {
                            std::string entryName = "##begin" + std::to_string(row);
                            glm::vec3 begin = data.beginPoint;
                            ImGui::PushItemWidth(-0.1);
                            if (ImGui::DragFloat3(entryName.c_str(), &begin[0],
                                0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
                            {
                                data.beginPoint = begin;
                            }
                            ImGui::PopItemWidth();
                        }
                        else if (column == 2)
                        {
                            std::string entryName = "##end" + std::to_string(row);
                            glm::vec3 end = data.endPoint;
                            ImGui::PushItemWidth(-0.1);
                            if (ImGui::DragFloat3(entryName.c_str(), &end[0],
                                0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
                            {
                                data.endPoint = end;
                            }
                            ImGui::PopItemWidth();
                        }
                        else
                        {
                            std::string entryName = "x##Remove" + std::to_string(row);
                            if (ImGui::SmallButton(entryName.c_str()))
                            {
                                removeIndex = row;
                            }
                        }
                    }
                }
                ImGui::EndTable();
            }

            ImGui::Spacing();
            if (ImGui::SmallButton("Add"))
            {
                if (lineData->Size() == 0)
                {
                    lineData->PushBack({{0,0,0}, {1,0,0}});
                }
                else
                {
                    renderer::LineData data = (*lineData)[lineData->Size()-1];
                    data.beginPoint = data.endPoint;
                    lineData->PushBack(data);
                }
            }

            ImGui::SameLine();
            if(ImGui::SmallButton("Remove All"))
            {
                lineData->Clear(); 
            }

            if (removeIndex != -1)
            {
                lineData->Erase(removeIndex, 1);
                removeIndex = -1;
            }
        }

        ImGui::Separator();
    }
}

void EntityProperties::ShowScriptComponent()
{
    if (ImGui::CollapsingHeader("Script Component"))
    {
        RemoveComponent(Component::Type::Script);

        scripting::ScriptComponent* component =
            dynamic_cast<scripting::ScriptComponent*>(
                selectedEntity->GetComponent(Component::Type::Script));

        ImGui::SeparatorText("Script");
        
        if (!availableScriptCached)
        {
            assetManager->GetAvailableScripts(availableScripts);
            availableScriptCached = true;
        }

        std::string currentScriptStr = component->script->GetResourcePath();

        const char* currentScriptPath = currentScriptStr.c_str();
        if (ImGui::BeginCombo("Script Path", currentScriptPath))
        {
            for (int n = 0; n < availableScripts.size(); n++)
            {
                const bool isSelected =
                    strcmp(availableScripts[n].c_str(), currentScriptPath) == 0;
                if (ImGui::Selectable(availableScripts[n].c_str(), isSelected))
                {
                    component->script->LoadSource(
                        availableScripts[n], selectedEntity);
                }

                // Set the initial focus to the selected mesh
                // when opening the combo
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button("Open in VS Code"))
        {
            std::string cmd = "code " + assetManager->GetWorkspacePath();
            if (system(cmd.c_str()))
            {
                Logger::Write(
                    "Failed to open VS Code.",
                    Logger::Warning, Logger::MsgType::Editor
                );
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Compile"))
        {
            if (component->script->HasScript())
            {
                component->script->LoadSource(
                    component->script->GetResourcePath(),
                    selectedEntity
                );
            }
        }

        ImGui::Separator();
    }
}

void EntityProperties::ShowDynamicBodyComponent()
{
    if (ImGui::CollapsingHeader("Dynamic Rigidbody Component"))
    {
        RemoveComponent(Component::Type::DynamicBody);

        physics::DynamicBodyComponent* component =
            dynamic_cast<physics::DynamicBodyComponent*>(
                selectedEntity->GetComponent(Component::Type::DynamicBody));
        physics::DynamicRigidbody * rigidbody = component->dynamicBody;

        ImGui::SeparatorText("Properties");
        {
            bool isKinematic = rigidbody->GetKinematic();
            if (ImGui::Checkbox("Is Kinematic", &isKinematic))
            {
                rigidbody->SetKinematic(isKinematic);
            }

            bool isGravity = rigidbody->GetGravity();
            if (ImGui::Checkbox("Is Gravity Affected", &isGravity))
            {
                rigidbody->SetGravity(isGravity);
            }

            float mass = rigidbody->GetMass();
            if (ImGui::DragFloat("Mass", &mass, 0.5f, 0.0f,
                FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
            {
                rigidbody->SetMass(mass);
            }

            glm::vec3 inertia = rigidbody->GetMassSpaceInertiaTensor();
            if (ImGui::DragFloat3("Inertia", &inertia[0], 0.5f, 0.0,
                FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
            {
                rigidbody->SetMassSpaceInertiaTensor(inertia);
            }

            float linearDamp = rigidbody->GetLinearDamping();
            if (ImGui::DragFloat("Linear Damp", &linearDamp, 0.01f, 0.0f,
                FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
            {
                rigidbody->SetLinearDamping(linearDamp);
            }

            float angularDamp = rigidbody->GetAngularDamping();
            if (ImGui::DragFloat("Angular Damp", &angularDamp, 0.01f, 0.0f,
                FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
            {
                rigidbody->SetAngularDamping(angularDamp);
            }
        }

        ImGui::SeparatorText("Shapes");
        {
            std::vector<physics::CollisionShape *> shapeList;
            rigidbody->GetShapes(shapeList);
            for (int i = 0; i < shapeList.size(); i++)
            {
                ImGuiTreeNodeFlags treeFlags =
                    ImGuiTreeNodeFlags_OpenOnArrow |
                    ImGuiTreeNodeFlags_OpenOnDoubleClick |
                    ImGuiTreeNodeFlags_SpanAvailWidth;

                switch (shapeList[i]->GetGeometryType())
                {
                case physics::GeometryType::eBOX:
                {
                    std::string treeName = std::to_string(i) + ". Box Shape";
                    if(ImGui::TreeNodeEx(treeName.c_str(), treeFlags))
                    {
                        physics::BoxGeometry box;
                        shapeList[i]->GetBoxGeometry(box);
                        glm::vec3 halfExtent =
                        {
                            box.halfExtents.x,
                            box.halfExtents.y,
                            box.halfExtents.z
                        };

                        if (ImGui::DragFloat3("Half Extent", &halfExtent[0],
                            0.01f, 0.01f, FLT_MAX, "%.3f",
                            ImGuiSliderFlags_AlwaysClamp))
                        {
                            box.halfExtents.x = halfExtent.x;
                            box.halfExtents.y = halfExtent.y;
                            box.halfExtents.z = halfExtent.z;

                            shapeList[i]->SetGeometry(box);
                        }

                        DrawPhysicsShapeCommon(shapeList[i]);

                        ImGui::TreePop();
                    }
                }
                    break;
                case physics::GeometryType::eSPHERE:
                {
                    std::string treeName = std::to_string(i) + ". Sphere Shape";
                    if(ImGui::TreeNodeEx(treeName.c_str(), treeFlags))
                    {
                        physics::SphereGeometry sphere;
                        shapeList[i]->GetSphereGeometry(sphere);

                        float radius = sphere.radius;

                        if (ImGui::DragFloat("Radius", &radius,
                            0.01f, 0.01f, FLT_MAX, "%.2f",
                            ImGuiSliderFlags_AlwaysClamp))
                        {
                            sphere.radius = radius;
                            shapeList[i]->SetGeometry(sphere);
                        }

                        DrawPhysicsShapeCommon(shapeList[i]);

                        ImGui::TreePop();
                    }
                }
                    break;

                case physics::GeometryType::eCAPSULE:
                {
                    std::string treeName = std::to_string(i) + ". Capsule Shape";
                    if(ImGui::TreeNodeEx(treeName.c_str(), treeFlags))
                    {
                        physics::BoxGeometry box;
                        shapeList[i]->GetBoxGeometry(box);
                        //FIXME: range not 0
                        glm::vec3 halfExtent =
                        {
                            box.halfExtents.x,
                            box.halfExtents.y,
                            box.halfExtents.z
                        };

                        if (ImGui::DragFloat3("Half Extent", &halfExtent[0],
                            0.01f, 0.0f, FLT_MAX, "%.3f",
                            ImGuiSliderFlags_AlwaysClamp))
                        {
                            box.halfExtents.x = halfExtent.x;
                            box.halfExtents.y = halfExtent.y;
                            box.halfExtents.z = halfExtent.z;

                            shapeList[i]->SetGeometry(box);
                        }

                        DrawPhysicsShapeCommon(shapeList[i]);

                        ImGui::TreePop();
                    }
                }
                    break;
                
                default:
                    break;
                }
    
            }

            ImGui::Spacing();
            if (ImGui::Button("Add.."))
                ImGui::OpenPopup("Add Shape");
            
            std::map<std::string, physics::GeometryType> shapeNames =
            {
                {"Box",     physics::GeometryType::eBOX    },
                {"Sphere",  physics::GeometryType::eSPHERE },
                {"Capsule", physics::GeometryType::eCAPSULE}
            };
            if (ImGui::BeginPopup("Add Shape"))
            {
                ImGui::SeparatorText("Shapes");
                for (auto& s: shapeNames)
                {
                    if (ImGui::Selectable(s.first.c_str()))
                    {
                        rigidbody
                            ->GetContext()
                            ->AddCollisionShape(rigidbody, s.second);
                    }
                }     
                ImGui::EndPopup();
            }
        }
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
        "Script Component",
        "Line Component",
        "Dynamic Rigidbody Component"
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
    else if (selectedComponent == "Script Component" &&
        !selectedEntity->HasComponent(Component::Type::Script))
    {
        selectedEntity->AddComponent(Component::Type::Script);
    }
    else if (selectedComponent == "Line Component" &&
        !selectedEntity->HasComponent(Component::Type::Line))
    {
        selectedEntity->AddComponent(Component::Type::Line);
    }
    else if (selectedComponent == "Dynamic Rigidbody Component" &&
        !selectedEntity->HasComponent(Component::Type::DynamicBody))
    {
        selectedEntity->AddComponent(Component::Type::DynamicBody);
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


void DrawPhysicsShapeCommon(physics::CollisionShape* shape)
{
    glm::mat4 transform;
    shape->GetLocalTransform(transform);

    if (ImGui::DragFloat3("Translation", &transform[3][0],
        0.01f, -FLT_MAX, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
    {
        shape->SetLocalTransform(transform);
    }

    glm::vec3 r; // local euler XYZ
    glm::vec3 rCached;

    glm::extractEulerAngleXYZ(
        glm::mat4(
            glm::normalize(transform[0]),
            glm::normalize(transform[1]),
            glm::normalize(transform[2]),
            transform[3]
        ),
        r[0], r[1], r[2]
    );

    rCached = r;
    if (ImGui::DragFloat3("Rotation", &r[0], 0.01f,
        -FLT_MAX, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
    {
        if (rCached[0] != r[0])
        {
            if (glm::abs(rCached[0] - r[0]) > 0.1f)
            {
                glm::mat4 newTransform = glm::eulerAngleXYZ(r[0], r[1], r[2]);
                newTransform[3] = transform[3];
                shape->SetLocalTransform(newTransform);
            }
            else
            {
                math::RotateAroundBasis0(transform, r[0] - rCached[0]);
                shape->SetLocalTransform(transform);
            }
        }
        else if (rCached[1] != r[1])
        {
            if (glm::abs(rCached[1] - r[1]) > 0.1f)
            {
                glm::mat4 newTransform = glm::eulerAngleXYZ(r[0], r[1], r[2]);
                newTransform[3] = transform[3];
                shape->SetLocalTransform(newTransform);
            }
            else
            {
                math::RotateAroundBasis1(transform, r[1] - rCached[1]);
                shape->SetLocalTransform(transform);
            }
        }
        else if (rCached[2] != r[2])
        {
            if (glm::abs(rCached[2] - r[2]) > 0.1f)
            {
                glm::mat4 newTransform = glm::eulerAngleXYZ(r[0], r[1], r[2]);
                newTransform[3] = transform[3];
                shape->SetLocalTransform(newTransform);
            }
            else
            {
                math::RotateAroundBasis2(transform, r[2] - rCached[2]);
                shape->SetLocalTransform(transform);
            }
        }
    }

    float staticFriction = shape->GetStaticFriction();
    if (ImGui::DragFloat("Static Friction", &staticFriction,
        0.001f, 0.0f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
    {
        shape->SetStaticFriction(staticFriction);   
    }

    float dynamicFriction = shape->GetDynamicFriction();
    if (ImGui::DragFloat("Dynamic Friction", &dynamicFriction,
        0.001f, 0.0f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
    {
        shape->SetDynamicFriction(dynamicFriction);   
    }

    float restitution = shape->GetRestitution();
    if (ImGui::DragFloat("Restitution", &restitution,
        0.001f, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
    {
        shape->SetRestitution(restitution);   
    }

    bool isTrigger = shape->GetTrigger();
    if (ImGui::Checkbox("Is Trigger", &isTrigger))
    {
        shape->SetTrigger(isTrigger);
    }
}