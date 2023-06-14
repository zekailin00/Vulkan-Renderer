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
    entity->SetName("editor");

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

void EditorApplication::OnUpdated(float ts)
{
    glm::mat4 view;
    static float totalTime = 0;
    totalTime += ts;

    view = glm::rotate(glm::mat4(1.0f), totalTime/1.0f, glm::vec3(1.0f, 0.0f, 0.0f));

    Entity* arrow = GetActiveScene(projectHandle)->GetEntityByName("arrow");
    arrow->SetLocalTransform(view);
}

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