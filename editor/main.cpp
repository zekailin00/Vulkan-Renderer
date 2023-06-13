#include "application.h"
#include "editor.h"

#include <iostream>

class EditorApplication: public Application
{
public:
    EditorApplication(std::string workspacePath)
        :Application(workspacePath){}

    void OnCreated() override;
    void OnUpdated(float ts) override;
    void OnDestroy() override;

    int editorHandle;
    int projectHandle;
    Scene* project;
};

void EditorApplication::OnCreated() 
{
    editorHandle = SetActiveScene(Scene::NewScene("editor"));
    {
        std::string workspace;
        Configuration::Get(CONFIG_WORKSPACE_PATH, workspace);
        
        project = Scene::LoadFromFile(
                workspace + "/" + SCENE_PATH + "/" +
                "newScene" + SCENE_EXTENSION, Scene::State::Editor
            );
        
        projectHandle = SetActiveScene(project);
    }

    Entity* entity = GetActiveScene(editorHandle)->NewEntity();
    entity->name = "editor";

    renderer::UIComponent* comp = (renderer::UIComponent*)
        entity->AddComponent(Component::Type::UI);
    
    renderer::UIBuildInfo info{};
    info.extent = {3840, 2160};
    info.imgui = Editor(project);
    info.editorUI = true;
    comp->ui = renderer::VulkanUI::BuildUI(info);
    
    renderer->SetWindowContent(comp->ui);
    // Entity* e = project->GetEntityByName("camera");
    // renderer::CameraComponent* camComp = (renderer::CameraComponent*)
    //     e->GetComponent(Component::Type::Camera);
    // renderer->SetWindowContent(camComp->camera);
}

void EditorApplication::OnUpdated(float ts) {}

void EditorApplication::OnDestroy()
{
    delete EraseActiveScene(editorHandle);
    delete EraseActiveScene(projectHandle);
}

int main(int argc, char** argv)
{
    EditorApplication app{"C:\\Users\\zekai\\Desktop\\ws-test\\scene-c++"};
    app.Run();
    return 0;
}