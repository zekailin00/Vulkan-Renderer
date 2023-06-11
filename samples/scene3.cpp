#include "application.h"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>


void UItest1()
{
    ImGui::Text("test test test test");
}


void UItest2()
{
    ImGui::Text("test test test test");
    ImGui::Text("test test test test");
    ImGui::Text("test test test test");
    ImGui::Text("test test test test");
}

class TestApp: public Application
{
public:
    std::shared_ptr<renderer::Camera> camera;
    std::shared_ptr<renderer::Mesh> mesh_test;
    std::shared_ptr<renderer::Mesh> mesh_monster;

    std::shared_ptr<renderer::Material> material_uv_test;
    std::shared_ptr<renderer::Material> material_color;

    std::shared_ptr<renderer::Texture> rough;
    std::shared_ptr<renderer::Texture> metal;

    Entity* arrowEntity;
    TestApp(std::string workspacePath)
        :Application(workspacePath){}

    Scene* m_scene;
    AssetManager* m_manager;

    void OnCreated() override;
    void OnUpdated(float ts) override;
    void OnDestroy() override;

};

void TestApp::OnCreated()
{
    m_manager = GetAssetManager();

    // {
    //     std::string workspace;
    //     Configuration::Get(CONFIG_WORKSPACE_PATH, workspace);
    //     SetActiveScene(
    //         Scene::LoadFromFile(
    //             workspace + "/" + SCENE_PATH + "/" +
    //             "newScene" + SCENE_EXTENSION, Scene::State::Editor
    //         )
    //     );
    // }

    {

        SetActiveScene(Scene::NewScene("newScene"));
        m_scene = GetActiveScene();

        Entity* entity = m_manager->ImportModelObj(
            "C:/Users/zekai/Desktop/quad.obj", m_scene);

        std::shared_ptr<renderer::Mesh> mesh =
            ((renderer::MeshComponent*)entity->GetComponent(
                Component::Type::Mesh))->mesh;

        std::shared_ptr<renderer::Material> material =
            m_manager->NewMaterial();
        std::shared_ptr<renderer::Texture> texture =
            m_manager->ImportTexture("C:/Users/zekai/Desktop/ws-test/img1.jpg");
        material->AddAlbedoTexture(texture);
        
        mesh->AddMaterial(material);

        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 3.0f, 0.5f)) * 
               glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));
        entity->SetLocalTransform(view);
    }

    {

        ((renderer::WireframeComponent*)m_scene->NewEntity()->AddComponent(
            Component::Type::Wireframe))->wireframe = 
            renderer::VulkanWireframeGenerator::GetCircle(
                glm::vec3(1, 3, -1), glm::vec3(0,0.5,1), 1, {1.0, 0, 0});

        ((renderer::WireframeComponent*)m_scene->NewEntity()->AddComponent(
            Component::Type::Wireframe))->wireframe = 
            renderer::VulkanWireframeGenerator::GetCircle(
                glm::vec3(1, 4, 1), glm::vec3(0,1,1), 0.5, {0, 1.0, 0});

        ((renderer::WireframeComponent*)m_scene->NewEntity()->AddComponent(
            Component::Type::Wireframe))->wireframe = 
            renderer::VulkanWireframeGenerator::GetCircle(
                glm::vec3(-1, 3, 1), glm::vec3(0,4,1), 1.5, {0, 0, 1.0});

        ((renderer::WireframeComponent*)m_scene->NewEntity()->AddComponent(
            Component::Type::Wireframe))->wireframe = 
            renderer::VulkanWireframeGenerator::GetCircle(
                glm::vec3(-1, 3, -1), glm::vec3(0,10,1), 1, {1.0, 1.0, 0});    
    }

    {
        Entity* entity = m_scene->NewEntity();

        renderer::CameraProperties prop{};
        prop.UseFrameExtent = false;
        this->camera = renderer::VulkanCamera::BuildCamera(prop);

        renderer::CameraComponent* camComp =
            (renderer::CameraComponent*)entity->AddComponent(Component::Type::Camera);

        camComp->camera = std::dynamic_pointer_cast<renderer::VulkanCamera>(this->camera);

        glm::mat4 view = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f))
                * glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f))
                * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.0f, 10));
        entity->SetLocalTransform(view);
    }

    {
        Entity* entity = m_manager->ImportModelObj(
            "C:/Users/zekai/Desktop/Vulkan-Renderer/resources/models/debug/arrow_debug.obj",
            m_scene
        );

        std::shared_ptr<renderer::Mesh> mesh =
            ((renderer::MeshComponent*)entity->GetComponent(
                Component::Type::Mesh))->mesh;

        std::shared_ptr<renderer::Material> material =
            m_manager->NewMaterial();
        material->SetAlbedo({1, 0, 0});
        mesh->AddMaterial(material);

        renderer::LightComponent* lightComp =
            (renderer::LightComponent*)entity->AddComponent(Component::Type::Light);
        lightComp->properties.color = {10, 5, 5};

        arrowEntity = entity;
        entity->name = "arrow";
    }
}

void TestApp::OnUpdated(float ts)
{

    // std::cout << "Frame frate: " << 1/ts << "| ts: " << ts << "\r"; 
    glm::mat4 view;
    static float totalTime = 0;
    totalTime += ts;

    view = glm::rotate(glm::mat4(1.0f), totalTime/1.0f, glm::vec3(1.0f, 0.0f, 0.0f));

    Entity* arrow = GetActiveScene()->GetEntityByName("arrow");
    arrow->SetLocalTransform(view);
}

void TestApp::OnDestroy()
{
    Scene* scene = GetActiveScene();
    std::string workspace;
    Configuration::Get(CONFIG_WORKSPACE_PATH, workspace);

    scene->SaveToFile(
        workspace + "/" + SCENE_PATH + "/" +
        scene->GetSceneName() + SCENE_EXTENSION
    );
    delete scene;
}

int main(int argv, char** argc)
{
    {   
        TestApp app("C:\\Users\\zekai\\Desktop\\ws-test\\test2");
        app.Run();
    }
}