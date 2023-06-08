#include "application.h"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>


using namespace renderer;


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
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Mesh> mesh_test;
    std::shared_ptr<Mesh> mesh_monster;

    std::shared_ptr<Material> material_uv_test;
    std::shared_ptr<Material> material_color;

    std::shared_ptr<Texture> rough;
    std::shared_ptr<Texture> metal;

    Node* vrCamNode = nullptr;
    Node* camNode = nullptr;
    Node* lightNode = nullptr;
    Node* lightDebug = nullptr;
    Node* lightNode2 = nullptr;
    Node* lightDebug2 = nullptr;
    Node *rusk;

    void OnCreated() override;
    void OnUpdated(float ts) override;
    void OnDestroy() override;

};

void TestApp::OnCreated()
{
    Node* root = renderer->CreateScene()->GetRootNode();
    std::unique_ptr<VulkanNode> node;
    Node* nodePtr;

    glm::mat4 view;

    {
        GltfModel model;
        model.LoadModel("/Users/zekai/Desktop/rusk.gltf");
        node = std::make_unique<VulkanNode>();
        nodePtr = root->AddChildNode(std::move(node));
        rusk = nodePtr->AddChildNode(model.GetNode());

        view = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
        view = glm::rotate(view, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(0.0f, -0.3f, 0.3f));
        nodePtr->SetTransform(view);
    }
  
    {
        node = std::make_unique<VulkanNode>();
        BuildMeshInfo meshInfo{};
        std::string path = "/Users/zekai/Desktop/quad.obj";
        renderer::ObjLoader(path, meshInfo.vertices, meshInfo.indices);
        std::shared_ptr<Mesh> uiMesh = VulkanMesh::BuildMesh(meshInfo);

        UIBuildInfo info{};
        info.extent = {720, 720};
        info.imgui = UItest1;

        std::shared_ptr<UI> ui = VulkanUI::BuildUI(info);
        std::shared_ptr<Texture> uiTexture = ui->GetTexture();

        MaterialProperties matProp{};
        matProp.albedoTexture = uiTexture;
        std::shared_ptr<Material> uiMat = VulkanMaterial::BuildMaterial(&matProp);

        uiMesh->AddMaterial(uiMat);
        node->ui = ui;
        node->SetMesh(uiMesh);
        nodePtr = root->AddChildNode(std::move(node));

        view = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 3.0f, 0.5f)) * 
               glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));
        nodePtr->SetTransform(view);
    }

    {
        node = std::make_unique<VulkanNode>();
        BuildMeshInfo meshInfo{};
        std::string path = "/Users/zekai/Desktop/quad.obj";
        renderer::ObjLoader(path, meshInfo.vertices, meshInfo.indices);
        std::shared_ptr<Mesh> uiMesh = VulkanMesh::BuildMesh(meshInfo);

        UIBuildInfo info{};
        info.extent = {720, 720};
        info.imgui = UItest1;

        std::shared_ptr<UI> ui = VulkanUI::BuildUI(info);
        std::shared_ptr<Texture> uiTexture = ui->GetTexture();

        MaterialProperties matProp{};
        matProp.albedoTexture = uiTexture;
        std::shared_ptr<Material> uiMat = VulkanMaterial::BuildMaterial(&matProp);

        uiMesh->AddMaterial(uiMat);
        node->ui = ui;
        node->SetMesh(uiMesh);
        nodePtr = root->AddChildNode(std::move(node));

        view = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, -1.0f)) * 
               glm::rotate(view, 3.14f, glm::vec3(0.0f, 1.0f, 0.0f)) *
               glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 1.0f));
        nodePtr->SetTransform(view);
    }


    {
        node = std::make_unique<VulkanNode>();
        BuildMeshInfo meshInfo{};
        std::string path = "/Users/zekai/Desktop/quad.obj";
        renderer::ObjLoader(path, meshInfo.vertices, meshInfo.indices);
        std::shared_ptr<Mesh> uiMesh = VulkanMesh::BuildMesh(meshInfo);

        UIBuildInfo info{};
        info.extent = {720, 360};
        info.imgui = UItest2;

        std::shared_ptr<UI> ui = VulkanUI::BuildUI(info);
        std::shared_ptr<Texture> uiTexture = ui->GetTexture();

        MaterialProperties matProp{};
        matProp.albedoTexture = uiTexture;
        std::shared_ptr<Material> uiMat = VulkanMaterial::BuildMaterial(&matProp);

        uiMesh->AddMaterial(uiMat);
        node->ui = ui;
        node->SetMesh(uiMesh);
        nodePtr = root->AddChildNode(std::move(node));


        view = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 3.0f, -0.5f)) * 
               glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 1.0f, 1.0f));
        nodePtr->SetTransform(view);
    }

    {
        Wireframe* generator = new VulkanWireframe();

        root->AddChildNode(generator->GetCircle(
            glm::vec3(1, 3, -1), glm::vec3(0,0.5,1), 1, {1.0, 0, 0}));

        root->AddChildNode(generator->GetCircle(
            glm::vec3(1, 4, 1), glm::vec3(0,1,1), 0.5, {0, 1.0, 0}));

        root->AddChildNode(generator->GetCircle(
            glm::vec3(-1, 3, 1), glm::vec3(0,4,1), 1.5, {0, 0, 1.0}));

        root->AddChildNode(generator->GetCircle(
            glm::vec3(-1, 3, -1), glm::vec3(0,10,1), 1, {1.0, 1.0, 0}));

        root->AddChildNode(generator->GetLine({2, -4, 0}, {2, 3, 0}));
        root->AddChildNode(generator->GetLine({2, 3, 0}, {-2, 3, 0}));
        root->AddChildNode(generator->GetLine({-2, 3, 0}, {-2, -4, 0}));

        root->AddChildNode(generator->GetLine({-2,  2, 0}, {2,  2, 0}, {1,0,0}));
        root->AddChildNode(generator->GetLine({-2,  1, 0}, {2,  1, 0}, {0,1,0}));
        root->AddChildNode(generator->GetLine({-2,  0, 0}, {2,  0, 0}, {0,0,1}));
        root->AddChildNode(generator->GetLine({-2, -4, 0}, {2, -4, 0}, {1,0,0}));

        root->AddChildNode(generator->GetSphere({2,1,2}, 1, {0,1,1}));
        root->AddChildNode(generator->GetAABB({2,2,2}, {3,5,1}));
        root->AddChildNode(generator->GetOBB(glm::mat4(1.0), {1,1,0}));

        root->AddChildNode(generator->GetAABB({-1,4,0}, {1,6,0}));

        
    }

    {
        node = std::make_unique<VulkanNode>();
        nodePtr = root->AddChildNode(std::move(node));

        CameraProperties prop{};
        prop.UseFrameExtent = false;
        this->camera = VulkanCamera::BuildCamera(prop);
        nodePtr->SetCamera(this->camera);

        view = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f))
                * glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f))
                * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.0f, 10));
        nodePtr->SetTransform(view);
        camNode = nodePtr;
    }

    {
        node = std::make_unique<VulkanNode>();
        nodePtr = root->AddChildNode(std::move(node));

        nodePtr->SetCamera(VulkanVrDisplay::BuildCamera());

        view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 3.0));
        nodePtr->SetTransform(view);
        vrCamNode = nodePtr;
    }

    {
        node = std::make_unique<VulkanNode>();
        nodePtr = vrCamNode->AddChildNode(std::move(node));

        BuildMeshInfo meshInfo{};
        std::string path = "resources/models/debug/arrow_debug.obj";
        renderer::ObjLoader(path, meshInfo.vertices, meshInfo.indices);
        mesh_test = VulkanMesh::BuildMesh(meshInfo);

        nodePtr->SetMesh(mesh_test);

        view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0));
        nodePtr->SetTransform(view);
    }


    {
        node = std::make_unique<VulkanNode>();
        nodePtr = root->AddChildNode(std::move(node));

        BuildMeshInfo meshInfo{};
        std::string path = "resources/models/debug/arrow_debug.obj";
        renderer::ObjLoader(path, meshInfo.vertices, meshInfo.indices);
        mesh_test = VulkanMesh::BuildMesh(meshInfo);

        nodePtr->SetMesh(mesh_test);
        MaterialProperties matProp{};
        matProp.albedo = {1, 0, 0};
        material_uv_test = VulkanMaterial::BuildMaterial(&matProp);
        mesh_test->AddMaterial(material_uv_test);

        view = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        nodePtr->SetTransform(view);
        lightDebug = nodePtr;

    }

    {
        node = std::make_unique<VulkanNode>();
        nodePtr = root->AddChildNode(std::move(node));

        renderer::LightProperties prop{};
        prop.color = {5, 5, 5};
        prop.type = DIRECTIONAL_LIGHT;
        std::shared_ptr<Light> light = VulkanLight::BuildLight(prop);

        nodePtr->SetLight(light);

        view = glm::rotate(glm::mat4(1.0f), 1.5f, glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        nodePtr->SetTransform(view);
        lightNode = nodePtr;
    }

    {
        node = std::make_unique<VulkanNode>();
        nodePtr = root->AddChildNode(std::move(node));

        renderer::LightProperties prop{};
        prop.color = {20, 5, 5};
        prop.type = DIRECTIONAL_LIGHT;
        std::shared_ptr<Light> light = VulkanLight::BuildLight(prop);

        nodePtr->SetLight(light);
        lightNode2 = nodePtr;
    }

    {
        node = std::make_unique<VulkanNode>();
        nodePtr = root->AddChildNode(std::move(node));

        BuildMeshInfo meshInfo{};
        std::string path = "resources/models/debug/arrow_debug.obj";
        renderer::ObjLoader(path, meshInfo.vertices, meshInfo.indices);
        mesh_test = VulkanMesh::BuildMesh(meshInfo);

        nodePtr->SetMesh(mesh_test);
        MaterialProperties matProp{};
        matProp.albedo = {0, 0, 1};
        material_uv_test = VulkanMaterial::BuildMaterial(&matProp);
        mesh_test->AddMaterial(material_uv_test);

        lightDebug2 = nodePtr;
    }
}

void TestApp::OnUpdated(float ts)
{

    // std::cout << "Frame frate: " << 1/ts << "| ts: " << ts << "\r"; 
    glm::mat4 view;
    static float totalTime = 0;
    totalTime += ts;

    view = glm::rotate(glm::mat4(1.0f), totalTime/1.0f, glm::vec3(1.0f, 0.0f, 0.0f));

    lightNode->SetTransform(view);
    lightDebug->SetTransform(view);

    view = glm::rotate(glm::mat4(1.0f), 0.7f, glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, totalTime/1.0f + 3.14f, glm::vec3(0.0f, 0.0f, 1.0f));
    lightNode2->SetTransform(view);
    lightDebug2->SetTransform(view);

    view = glm::rotate(glm::mat4(1.0f), totalTime/3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    //rusk->SetTransform(view);

    view = glm::rotate(glm::mat4(1.0f), 0/3.0f, glm::vec3(1.0f, 0.0f, 0.0f))
                * glm::rotate(glm::mat4(1.0f), totalTime/3.0f, glm::vec3(0.0f, 1.0f, 0.0f))
                * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.0f, 10));
    camNode->SetTransform(view);

}

void TestApp::OnDestroy()
{

}

int main(int argv, char** argc)
{
    {   
        TestApp app{};
        app.Run();
    }
}