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
};

void EditorApplication::OnCreated() 
{
    SetActiveScene(Scene::NewScene("newScene"));
    Entity* entity = GetActiveScene()->NewEntity();
    entity->name = "editor";

    renderer::UIComponent* comp = (renderer::UIComponent*)
        entity->AddComponent(Component::Type::UI);
    
    renderer::UIBuildInfo info{};
    info.extent = {3840, 2160};
    info.imgui = Editor();
    info.editorUI = true;
    comp->ui = renderer::VulkanUI::BuildUI(info);
    
    renderer->SetWindowContent(comp->ui);
}

void EditorApplication::OnUpdated(float ts) {}

void EditorApplication::OnDestroy()
{
    delete GetActiveScene();
}

int main(int argc, char** argv)
{
    EditorApplication app{"C:/Users/zekai/Desktop/ws-test/editor-t1"};
    app.Run();
    return 0;
}