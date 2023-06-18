#pragma once

#include <map>
#include <string>
#include <memory>
#include <filesystem>

#include "vulkan_material.h"
#include "vulkan_mesh.h"
#include "vulkan_texture.h"

#include "renderer_asset_manager.h"
#include "core_asset_manager.h"

#include "scene.h"

#define PROJECT_FILE    "workspace.slproj"

#define TEXTURE_PATH    "resources/textures"
#define MESH_PATH       "resources/meshes"
#define MATERIAL_PATH   "resources/materials"
#define MODEL_PATH      "resources/models"
#define SCENE_PATH      "resources/scenes"

#define MATERIAL_EXTENSION   ".slmat"
#define TEXTURE_EXTENSION    ".sltex"
#define MESH_EXTENSION       ".slmsh"
#define MESH_DATA_EXTENSION  ".slmshd"
#define MODEL_EXTENSION      ".slmod"
#define SCENE_EXTENSION      ".slscn"

class AssetManager:
    public renderer::IRendererAssetManager,
    public ICoreAssetManager
{
public:
    static AssetManager* OpenProject(std::string workspacePath);
    static AssetManager* NewProject(std::string workspacePath);
    void SaveToFilesystem();
    void DestroyResources();

    std::shared_ptr<renderer::Material> NewMaterial();
    std::shared_ptr<renderer::Texture> ImportTexture(std::string path);

    Entity* ImportModelObj(std::string path, Scene* scene);
    // Entity* ImportModelGlft(std::string path, Scene* scene);
    

    /**
     * @brief Get the Material object
     * 
     * @param path Relative path.
     * @return std::shared_ptr<renderer::Material> 
     */
    std::shared_ptr<renderer::Material> GetMaterial(std::string path) override;

    /**
     * @brief Get the Mesh object
     * 
     * @param path Relative path.
     * @return std::shared_ptr<renderer::Mesh> 
     */
    std::shared_ptr<renderer::Mesh> GetMesh(std::string path) override;

    /**
     * @brief Get the Texture object
     * 
     * @param path Relative path.
     * @return std::shared_ptr<renderer::Texture> 
     */
    std::shared_ptr<renderer::Texture> GetTexture(std::string path) override;

    bool IsWorkspaceInitialized() override {return initialized;}
    std::string GetWorkspacePath() {return workspacePath;}
    
    void GetAvailableMeshes(std::vector<const char*>& meshPaths);
    void GetAvailableMaterials(std::vector<const char*>& materialPaths);
    void GetAvailableTextures(std::vector<const char*>& texturePaths);
    void GetAvailableScenes(std::vector<std::string>& scenePaths);

    std::string GetScenePath(std::string sceneName) override;

    ~AssetManager()
    {
        SaveToFilesystem();
        DestroyResources();
    }

private:
    AssetManager() = default;

    AssetManager(const AssetManager&) = delete;
    const AssetManager& operator=(const AssetManager&) = delete;

    bool InitializeWorkspace(std::string workspacePath, bool isNew);
    void LoadWorkspace();
    void CreateWorkspace();

    std::shared_ptr<renderer::Material> LoadMaterial(std::filesystem::path path);
    bool StoreMaterial(std::shared_ptr<renderer::Material> material);

    std::shared_ptr<renderer::Texture> LoadTexture(std::filesystem::path path);
    bool StoreTexture(std::shared_ptr<renderer::Texture> texture);

    std::shared_ptr<renderer::Mesh> LoadMesh(std::filesystem::path path);
    bool StoreMesh(std::shared_ptr<renderer::Mesh> mesh);

private:
    std::map<std::string, std::shared_ptr<renderer::Material>> materialList;
    std::map<std::string, std::shared_ptr<renderer::Mesh>> meshList;
    std::map<std::string, std::shared_ptr<renderer::Texture>> textureList;

    std::string workspacePath;
    bool initialized = false;
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