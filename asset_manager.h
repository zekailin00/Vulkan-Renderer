#pragma once

#include <map>
#include <string>
#include <memory>
#include <filesystem>

#include "vulkan_material.h"
#include "vulkan_mesh.h"
#include "vulkan_texture.h"

#include "renderer_assets.h"

#include "scene.h"

#define TEXTURE_PATH    "resources/textures"
#define MESH_PATH       "resources/meshes"
#define MATERIAL_PATH   "resources/materials"
#define MODEL_PATH      "resources/models"

#define MATERIAL_EXTENSION ".slmat"
#define TEXTURE_EXTENSION ".sltex"
#define MESH_EXTENSION ".slmesh"
#define MODEL_EXTENSION ".slmod"

class AssetManager: public renderer::AssetManager
{
    void InitializeWorkspace(std::string path);

    std::shared_ptr<renderer::Material> NewMaterial();
    std::shared_ptr<renderer::Texture> ImportTexture(std::string path);

    bool ImportModelObj(std::string path, Scene* scene);
    Entity* AddModelObjToScene(std::string path, Scene* scene);
    
    bool ImportModelGlft(std::string path, Scene* scene);
    Entity* AddModelGltfToScene(std::string path, Scene* scene);

    std::shared_ptr<renderer::Material> GetMaterial(std::string path) override;
    std::shared_ptr<renderer::Mesh> GetMesh(std::string path) override;
    std::shared_ptr<renderer::Texture> GetTexture(std::string path) override;

private:
    std::map<std::string, std::shared_ptr<renderer::Material>> materialList;
    std::map<std::string, std::shared_ptr<renderer::Mesh>> meshList;
    std::map<std::string, std::shared_ptr<renderer::Texture>> textureList;

private:
    void LoadWorkspace();
    void CreateWorkspace();

    std::shared_ptr<renderer::Material> LoadMaterial(std::filesystem::path path);
    bool StoreMaterial(std::shared_ptr<renderer::Material> material);

    std::shared_ptr<renderer::Texture> LoadTexture(std::filesystem::path path);
    bool StoreTexture(std::shared_ptr<renderer::Texture> texture);

    std::shared_ptr<renderer::Mesh> LoadMesh(std::filesystem::path path);
    bool StoreMesh(std::shared_ptr<renderer::Texture> texture);

private:
    std::string workspacePath;
    unsigned int materialCounter = 0;
};

struct MeshFile
{
    unsigned int indexSize;
    unsigned int vertexSize;

    static void Store(
        std::string fullPath,
        const std::vector<unsigned int>& modelIndices,
        const std::vector<renderer::Vertex>& modelVertices
    );
    static void Load(std::string fullPath,
        std::vector<unsigned int>& modelIndices,
        std::vector<renderer::Vertex>& modelVertices
    );
};