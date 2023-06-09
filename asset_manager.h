#pragma once

#include <map>
#include <string>
#include <memory>

#include "vulkan_material.h"
#include "vulkan_mesh.h"
#include "vulkan_texture.h"

#include "scene.h"

#define MATERIAL_EXTENSION ".slmat"


class AssetManager
{
    void InitializeWorkspace(std::string path);

    std::shared_ptr<renderer::Material> NewMaterial();
    std::shared_ptr<renderer::Texture> ImportTexture(std::string path);

    void ImportModelObj(std::string path);
    void ImportModelGlft(std::string path);
    
    Entity* AddModelObjToScene(std::string path, Scene* scene);
    Entity* AddModelGltfToScene(std::string path, Scene* scene);

    std::shared_ptr<renderer::Material> GetMaterial(std::string path);
    std::shared_ptr<renderer::Mesh> GetMesh(std::string path);
    std::shared_ptr<renderer::Texture> GetTexture(std::string path);

private:
    std::map<std::string, std::shared_ptr<renderer::Material>> materialList;
    std::map<std::string, std::shared_ptr<renderer::Mesh>> meshList;
    std::map<std::string, std::shared_ptr<renderer::Texture>> textureList;

private:
    void LoadWorkspace();
    void CreateWorkspace();

    std::shared_ptr<renderer::Material> LoadMaterial(Json::Value& json);
    bool StoreMaterial(std::shared_ptr<renderer::Material> material);

private:
    std::string workspacePath;
    unsigned int materialCounter = 0;
};

#define TEXTURE_PATH    "resources/textures"
#define MESH_PATH       "resources/meshes"
#define MATERIAL_PATH   "resources/materials"
#define MODEL_PATH      "resources/models"
