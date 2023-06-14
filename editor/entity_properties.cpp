#include "entity_properties.h"


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
        // char str0[128];
        // strcpy(str0, identifier.name.c_str());
        // ImGui::Text("Name: "); ImGui::SameLine();
        // if (ImGui::InputText("##identifier", str0, IM_ARRAYSIZE(str0), ImGuiInputTextFlags_EnterReturnsTrue))
        //     identifier.name = str0;
        ImGui::Text("Test test test");
        ImGui::Separator();
    }

    // TransformComponent& transform = context->ShowProperties.GetComponent<TransformComponent>();
    // if (ImGui::CollapsingHeader("Transform Component"))
    // {
    //     ImGui::Text("Local Translation: ");
    //     ImGui::InputFloat3("##translation", reinterpret_cast<float*>(&transform.translation), 
    //                         "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
    //     ImGui::Text("Local Scale: ");
    //     ImGui::InputFloat3("##scale", reinterpret_cast<float*>(&transform.scale), 
    //                         "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
    //     ImGui::Text("Local Rotation: ");
    //     ImGui::InputFloat3("##rotation", reinterpret_cast<float*>(&transform.rotation), 
    //                         "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);

    //     ImGui::Separator();
    // }

    // if (context->ShowProperties.HasComponent<MeshComponent>())
    // {
    //     ShowMeshComponent();
    // }


    // if (context->ShowProperties.HasComponent<JavascriptComponent>())
    // {
    //     ShowScriptComponent();
    // }

    // if(context->ShowProperties.HasComponent<LightComponent>())
    // {
    //     LightComponent& light = context->ShowProperties.GetComponent<LightComponent>();
    //     if (ImGui::CollapsingHeader("Light Component"))
    //     {
    //         ImGui::Text("Ambient Light: ");
    //         ImGui::InputFloat3("##Ambient", reinterpret_cast<float*>(&light.ambient), 
    //                             "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
    //         ImGui::Text("Diffuse Light: ");
    //         ImGui::InputFloat3("##Diffuse", reinterpret_cast<float*>(&light.diffuse), 
    //                             "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
            
    //         ImGui::Separator();
    //     }
    // }

    // if(context->ShowProperties.HasComponent<CameraComponent>())
    // {
    //     CameraComponent& camera = context->ShowProperties.GetComponent<CameraComponent>();
    //     if (ImGui::CollapsingHeader("Camera Component"))
    //     {
    //         ImGui::Text("Camera");
    //         ImGui::Separator();
    //     }
    // }

    
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