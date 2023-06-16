#include "application.h"
#include "editor.h"


class EditorApplication: public Application
{
public:
    void OnCreated() override;
    void OnUpdated(float ts) override;
    void OnDestroy() override;

    int editorHandle = -1;
    Editor* editor = nullptr;
};

void EditorApplication::OnCreated() 
{
    editor = new Editor(this);
    editorHandle = SetActiveScene(Scene::NewScene("editor", nullptr));

    Entity* entity = GetActiveScene(editorHandle)->NewEntity();
    entity->SetName("editor");
    renderer::UIComponent* comp = (renderer::UIComponent*)
        entity->AddComponent(Component::Type::UI);

    renderer::UIBuildInfo info{};
    info.editorUI = true;
    info.imgui = [this](){this->editor->DrawUI();};
    comp->ui = renderer::VulkanUI::BuildUI(info);
    
    renderer->SetWindowContent(comp->ui);
}

void EditorApplication::OnUpdated(float ts) {}

void EditorApplication::OnDestroy()
{
    delete EraseActiveScene(editorHandle);
    delete editor;
}

int main(int argc, char** argv)
{
    EditorApplication app{};
    app.Run();
    return 0;
}