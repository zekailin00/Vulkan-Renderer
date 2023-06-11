#include "application.h"

#include <iostream>


class TestApp: public Application
{
public:
    TestApp(std::string workspacePath)
        :Application(workspacePath){}

protected:
    void OnCreated() override;
    void OnUpdated(float ts) override{};
    void OnDestroy() override;
};


void TestApp::OnCreated()
{
    std::cout << "hello" << std::endl;
    SetActiveScene(Scene::NewScene("newScene"));

    Configuration::Set(CONFIG_WORKSPACE_PATH,
        "C:\\Users\\zekai\\Desktop\\ws-test\\test22");

    AssetManager* manager = GetAssetManager();
    std::shared_ptr<renderer::Texture> texture1_1 =
        manager->ImportTexture("C:/Users/zekai/Desktop/ws-test/img1.jpg");
    std::shared_ptr<renderer::Texture> texture1_2 =
        manager->ImportTexture("C:/Users/zekai/Desktop/ws-test/img1.jpg");

    std::shared_ptr<renderer::Texture> texture2_1 =
        manager->ImportTexture("C:/Users/zekai/Desktop/ws-test/img2.jpg");
    std::shared_ptr<renderer::Texture> texture3_1 =
        manager->ImportTexture("C:/Users/zekai/Desktop/ws-test/img3.jpg");

    std::shared_ptr<renderer::Material> material1 = manager->NewMaterial();
    std::shared_ptr<renderer::Material> material2 = manager->NewMaterial();
    std::shared_ptr<renderer::Material> material3 = manager->NewMaterial();

    material1->AddAlbedoTexture(texture1_1);
    material1->AddMetallicTexture(texture1_2);
    material1->AddRoughnessTexture(texture1_1);
    material1->AddNormalTexture(texture1_2);

    material2->AddAlbedoTexture(texture2_1);
    material2->AddMetallicTexture(texture2_1);
    
    manager->SaveToFilesystem();
}

void TestApp::OnDestroy()
{
    delete GetActiveScene();
}

int main(int argv, char** argc)
{
    {   
        TestApp app("C:\\Users\\zekai\\Desktop\\ws-test\\");
        app.Run();
    }
    return 0;
}