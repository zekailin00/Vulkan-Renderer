#include "application.h"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "vulkan_renderer/loaders/gltfloader.h"


using namespace renderer;


void ObjLoader(std::string& meshPath,
    std::vector<renderer::Vertex>& modelVertices,
    std::vector<unsigned int>& modelIndices);

class TestApp: public Application
{
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Mesh> mesh_test;
    std::shared_ptr<Mesh> mesh_monster;

    std::shared_ptr<Material> material_uv_test;
    std::shared_ptr<Material> material_color;

    Node* camNode = nullptr;
    Node* lightNode = nullptr;
    Node* lightDebug = nullptr;
    Node* lightNode2 = nullptr;
    Node* lightDebug2 = nullptr;

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
        model.LoadModel("/Users/zekailin00/Desktop/rusk.json");
        Node *rusk = root->AddChildNode(std::move(model.node));

        view = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f));
        view = glm::rotate(view, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, 2.8f, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(0.0f, -0.3f, 0.3f));
        rusk->SetTransform(view);
    }

    {
        node = std::make_unique<VulkanNode>();
        nodePtr = root->AddChildNode(std::move(node));

        CameraProperties prop{};
        this->camera = VulkanCamera::BuildCamera(prop);
        nodePtr->SetCamera(this->camera);

        view = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, 2.8f, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(0.0f, 2.0f, 10));
        nodePtr->SetTransform(view);
        camNode = nodePtr;
    }

    {
        node = std::make_unique<VulkanNode>();
        nodePtr = root->AddChildNode(std::move(node));

        BuildMeshInfo meshInfo{};
        std::string path = "resources/models/ext/test.obj";
        ObjLoader(path, meshInfo.vertices, meshInfo.indices);
        mesh_test = VulkanMesh::BuildMesh(meshInfo);

        nodePtr->SetMesh(mesh_test);
        MaterialProperties matProp{};
        matProp.albedoTexture = VulkanTexture::GetDefaultTexture();
        matProp.metallic = 0.8f;
        matProp.smoothness = 0.5f;
        material_uv_test = VulkanMaterial::BuildMaterial(&matProp);
        mesh_test->AddMaterial(material_uv_test);

        view = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(0.0f, 3.0f, 2.0f));
        nodePtr->SetTransform(view);
    }

    {
        node = std::make_unique<VulkanNode>();
        nodePtr = root->AddChildNode(std::move(node));

        BuildMeshInfo meshInfo{};
        std::string path = "resources/models/debug/arrow_debug.obj";
        ObjLoader(path, meshInfo.vertices, meshInfo.indices);
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
        prop.color = {30, 5, 5};
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

        BuildMeshInfo meshInfo{};
        std::string path = "resources/models/ext/armadillo.obj";
        ObjLoader(path, meshInfo.vertices, meshInfo.indices);
        mesh_monster = VulkanMesh::BuildMesh(meshInfo);

        nodePtr->SetMesh(mesh_monster);
        MaterialProperties matProp{};
        matProp.albedo = {0, 0.8, 0.8};
        matProp.metallic = 0.0f;
        matProp.smoothness = 1.0f;
        material_color = VulkanMaterial::BuildMaterial(&matProp);
        mesh_monster->AddMaterial(material_color);

        view = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(3.0f, 1.0f, -1.0f));
        nodePtr->SetTransform(view);
    }

    {
        node = std::make_unique<VulkanNode>();
        nodePtr = root->AddChildNode(std::move(node));

        BuildMeshInfo meshInfo{};
        std::string path = "resources/models/ext/monkey.obj";
        ObjLoader(path, meshInfo.vertices, meshInfo.indices);
        mesh_monster = VulkanMesh::BuildMesh(meshInfo);

        nodePtr->SetMesh(mesh_monster);
        MaterialProperties matProp{};
        matProp.albedo = {0, 0.8, 0.8};
        material_color = VulkanMaterial::BuildMaterial(&matProp);
        mesh_monster->AddMaterial(material_color);

        view = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(-3.0f, 1.0f, -1.0f));
        nodePtr->SetTransform(view);
    }

    {
        node = std::make_unique<VulkanNode>();
        nodePtr = root->AddChildNode(std::move(node));

        renderer::LightProperties prop{};
        prop.color = {5, 5, 30};
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
        ObjLoader(path, meshInfo.vertices, meshInfo.indices);
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

    std::cout << "Frame frate: " << 1/ts << "| ts: " << ts << "\r"; 
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
}

void TestApp::OnDestroy()
{

}

int main(int argv, char** argc)
{
    TestApp app{};
    app.Run();
    std::cout << "Hello world." << std::endl;
}


void ObjLoader(std::string& meshPath,
    std::vector<renderer::Vertex>& modelVertices,
    std::vector<unsigned int>& modelIndices)
{
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning, error;

    if (!tinyobj::LoadObj(&attributes, &shapes, &materials,
        &warning, &error, meshPath.c_str()))
    {
        throw;
    }

    for (tinyobj::shape_t shape : shapes) {
        for (const tinyobj::index_t& index : shape.mesh.indices) {
            renderer::Vertex vertex{};

            int vertIndex = index.vertex_index;
            int normIndex = index.normal_index;
            int texIndex = index.texcoord_index; 

            vertex.Position = {
                attributes.vertices[vertIndex * 3 + 0],
                attributes.vertices[vertIndex * 3 + 1],
                attributes.vertices[vertIndex * 3 + 2],
            };

            vertex.Normal = {
                attributes.normals[normIndex * 3 + 0],
                attributes.normals[normIndex * 3 + 1],
                attributes.normals[normIndex * 3 + 2],
            };


            if (texIndex < 0)
            {
                vertex.TexCoords = {0, 0};
            } else {
                vertex.TexCoords = {
                    attributes.texcoords[texIndex * 2 + 0],
                    1 - attributes.texcoords[texIndex * 2 + 1],
                };
            }

            modelVertices.push_back(vertex);
            modelIndices.push_back(static_cast<unsigned int>(modelIndices.size()));
        }
    }
}