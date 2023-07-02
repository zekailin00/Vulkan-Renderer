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
#include "script_asset_manager.h"

#include "scene.h"

#define PROJECT_FILE    "workspace.slproj"

#define TEXTURE_PATH    "resources/textures"
#define MESH_PATH       "resources/meshes"
#define MATERIAL_PATH   "resources/materials"
#define MODEL_PATH      "resources/models"
#define SCENE_PATH      "resources/scenes"
#define SCRIPT_PATH     "resources/scripts"

#define MATERIAL_EXTENSION      ".slmat"
#define TEXTURE_EXTENSION       ".sltex"
#define TEXTURE_DATA_EXTENSION  ".jpg"
#define MESH_EXTENSION          ".slmsh"
#define MESH_DATA_EXTENSION     ".slmshd"
#define MODEL_EXTENSION         ".slmod"
#define SCENE_EXTENSION         ".slscn"
#define SCRIPT_EXTENSION        ".js"

class AssetManager:
    public renderer::IRendererAssetManager,
    public scripting::IScriptAssetManager,
    public ICoreAssetManager
{
public:
    /**
     * @brief Open a project located at the path
     * 
     * @param workspacePath Absolute path to the workspace folder
     * @return AssetManager* 
     */
    static AssetManager* OpenProject(std::string workspacePath);

    /**
     * @brief Create a new project located at the path
     * 
     * @param workspacePath Absolute path to the new workspace folder
     * @return AssetManager* 
     */
    static AssetManager* NewProject(std::string workspacePath);

    /**
     * @brief Save all changes in memory back to filesystem.
     * 
     */
    void SaveToFilesystem();

    /**
     * @brief Destory all resources in memory.
     * It is automatically called when AssetManager is destoryed.
     * Because all resources are reference counted,
     * if a resource is still used by other parts of the system
     * (scene graph, renderer), it wouldn't be freed immediately
     * when Assetmanager is destroyed. 
     * 
     */
    void DestroyResources();

    std::shared_ptr<renderer::Material> NewMaterial();

    /**
     * @brief Import a texture into the current workspace.
     * 
     * @param path Absolute path to a valid image file.
     * @return std::shared_ptr<renderer::Texture> 
     */
    std::shared_ptr<renderer::Texture> ImportTexture(std::string path);

    Entity* ImportModelObj(std::string path, Scene* scene);
    Entity* ImportModelGltf(std::string path, Scene* scene);

    // bool ImportModelGlft(std::string path);
    // Entity* AddModelToScene(std::string path, Scene* scene);

    /**
     * @brief Get the Material object
     * 
     * @param path Path relative to workspace directory.
     * @return std::shared_ptr<renderer::Material> 
     */
    std::shared_ptr<renderer::Material> GetMaterial(std::string path) override;

    /**
     * @brief Get the Mesh object
     * 
     * @param path Path relative to workspace directory.
     * @return std::shared_ptr<renderer::Mesh> 
     */
    std::shared_ptr<renderer::Mesh> GetMesh(std::string path) override;

    /**
     * @brief Get the Texture object
     * 
     * @param path Path relative to workspace directory.
     * @return std::shared_ptr<renderer::Texture> 
     */
    std::shared_ptr<renderer::Texture> GetTexture(std::string path) override;

    /**
     * @brief Get the Javascript source code.
     * 
     * @param resourcePath Path relative to workspace directory.
     * @param source source code is written into the variable
     * @return Return true if loaded successfully
     */
    bool GetSourceCode(std::string resourcePath, std::string& source) override;

    bool NewSourceCode(std::string fileName) override;

    bool IsWorkspaceInitialized() override {return initialized;}
    std::string GetWorkspacePath() override {return workspacePath;}
    
    void GetAvailableMeshes(std::vector<const char*>& meshPaths) override;
    void GetAvailableMaterials(std::vector<const char*>& materialPaths) override;
    void GetAvailableTextures(std::vector<const char*>& texturePaths) override;
    void GetAvailableScenes(std::vector<std::string>& scenePaths) override;
    void GetAvailableScripts(std::vector<std::string>& scriptPaths) override;

    std::string GetScenePath(std::string sceneName) override;
    std::string GetTexturePath(std::string textureName) override;
    std::string GetMaterialPath(std::string materialName) override;
    std::string GetMeshPath(std::string meshName) override;
    std::string GetScriptPath(std::string scriptName) override;

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