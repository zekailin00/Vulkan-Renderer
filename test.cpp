#include "application.h"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

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

    void OnCreated() override;
    void OnUpdated() override;
    void OnDestroy() override;

};

void TestApp::OnCreated()
{
    Node* root = renderer->CreateScene()->GetRootNode();
    std::unique_ptr<VulkanNode> node;
    Node* nodePtr;

    glm::mat4 view;

    {
        node = std::make_unique<VulkanNode>();
        nodePtr = root->AddChildNode(std::move(node));

        CameraProperties prop{};
        this->camera = VulkanCamera::BuildCamera(prop);
        nodePtr->SetCamera(this->camera);

        view = glm::rotate(glm::mat4(1.0f), -0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(-2.0f, 2.0f, 10));
        nodePtr->SetTransform(view);
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
        matProp.albedo = {1, 0.5, 0.5};
        material_uv_test = VulkanMaterial::BuildMaterial(&matProp);
        mesh_test->AddMaterial(material_uv_test);

        view = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(0.0f, 1.0f, -1.0f));
        nodePtr->SetTransform(view);
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
        // matProp.albedoTexture = VulkanTexture::GetDefaultTexture();
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
        // matProp.albedoTexture = VulkanTexture::GetDefaultTexture();
        material_color = VulkanMaterial::BuildMaterial(&matProp);
        mesh_monster->AddMaterial(material_color);

        view = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(-3.0f, 1.0f, -1.0f));
        nodePtr->SetTransform(view);
    }


}

void TestApp::OnUpdated()
{
    // static int i = 0;
    // i++;
    // std::cout << "running: " << i << std::endl; 
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