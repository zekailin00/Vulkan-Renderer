#include "asset_manager.h"

#include "configuration.h"
#include "serialization.h"
#include "validation.h"
#include "filesystem.h"
#include "logger.h"

#include "obj_loader.h"

#include "mesh_component.h"

#include <string>
#include <stb/stb_image.h>


AssetManager* AssetManager::OpenProject(std::string workspacePath)
{
    AssetManager* manager = new AssetManager();
    if (manager->InitializeWorkspace(workspacePath, false))
    {
        return manager;
    }

    delete manager;
    return nullptr;
}

AssetManager* AssetManager::NewProject(std::string workspacePath)
{
    AssetManager* manager = new AssetManager();
    if (manager->InitializeWorkspace(workspacePath, true))
    {
        return manager;
    }

    delete manager;
    return nullptr;
}

bool AssetManager::InitializeWorkspace(std::string workspacePath, bool isNew)
{
    this->workspacePath = workspacePath;

    if (!Filesystem::IsAbsolute(workspacePath))
    {
        Logger::Write(
            "Workspace path format is not supported.",
            Logger::Level::Warning, Logger::MsgType::Platform
        );
        return false;
    }

    if (Filesystem::IsRegularFile(workspacePath))
    {
        Logger::Write(
            "Workspace path format is not supported.",
            Logger::Level::Warning, Logger::MsgType::Platform
        );
        return false;
    }

    if (Filesystem::IsDirectory(workspacePath))
    {
        if (!Filesystem::IsRegularFile(workspacePath  + "/" + PROJECT_FILE))
        {
            Logger::Write(
                "The directory is not a valid project",
                Logger::Level::Warning, Logger::MsgType::Platform
            );
            return false;
        }

        if (isNew)
        {
            Logger::Write(
                "The project is already created!",
                Logger::Level::Warning, Logger::MsgType::Platform
            );
            return false;
        }

        LoadWorkspace();
    }
    else
    {
        if (!isNew)
        {
            Logger::Write(
                "The project is not yet created!",
                Logger::Level::Warning, Logger::MsgType::Platform
            );
            return false;
        }

        CreateWorkspace();
    }

    initialized = true;
    return true;
}

void AssetManager::CreateWorkspace()
{
    Filesystem::CreateDirectory(workspacePath + "/" + TEXTURE_PATH);
    Filesystem::CreateDirectory(workspacePath + "/" + MESH_PATH);
    Filesystem::CreateDirectory(workspacePath + "/" + MATERIAL_PATH);
    Filesystem::CreateDirectory(workspacePath + "/" + MODEL_PATH);
    Filesystem::CreateDirectory(workspacePath + "/" + SCENE_PATH);

    std::ofstream out;
    out.open(workspacePath + "/" + PROJECT_FILE);
    out.write("record", 7);
    out.close();
}

void AssetManager::LoadWorkspace()
{
    std::vector<std::filesystem::path> entries;

    Filesystem::GetDirectoryEntries(workspacePath + "/" + TEXTURE_PATH, entries);
    for (auto& path: entries)
    {
        if (path.extension().string() == TEXTURE_EXTENSION)
        {
            std::shared_ptr<renderer::Texture> texture = LoadTexture(path);
            std::string relativePath = Filesystem::RemoveParentPath(
                path.string(), workspacePath
            );

            Filesystem::ToUnixPath(relativePath);
            textureList[relativePath] = texture;
        }
    }

    Filesystem::GetDirectoryEntries(workspacePath + "/" + MATERIAL_PATH, entries);
    for (auto& path: entries)
    {
        if (path.extension().string() == MATERIAL_EXTENSION)
        {
            std::shared_ptr<renderer::Material> material = LoadMaterial(path);
            std::string relativePath = Filesystem::RemoveParentPath(
                path.string(), workspacePath
            );

            Filesystem::ToUnixPath(relativePath);
            materialList[relativePath] = material;
        }
    }

    Filesystem::GetDirectoryEntries(workspacePath + "/" + MESH_PATH, entries);
    for (auto& path: entries)
    {
        if (path.extension().string() == MESH_EXTENSION)
        {
            std::shared_ptr<renderer::Mesh> mesh = LoadMesh(path);
            std::string relativePath = Filesystem::RemoveParentPath(
                path.string(), workspacePath
            );

            Filesystem::ToUnixPath(relativePath);
            meshList[relativePath] = mesh;
        }
    }
}

std::shared_ptr<renderer::Material> AssetManager::NewMaterial()
{
    renderer::MaterialProperties properties{};

    std::string wsRelativePath = MATERIAL_PATH;
    wsRelativePath = wsRelativePath + "/material" + MATERIAL_EXTENSION;

    std::string finalWsFullPath = Filesystem::GetUnusedFilePath(
        workspacePath + "/" + wsRelativePath
    );
    std::string finalWsRelativePath = Filesystem::RemoveParentPath(
        finalWsFullPath, workspacePath
    );

    properties.resourcePath = finalWsRelativePath;

    std::shared_ptr<renderer::Material> material =
        renderer::VulkanMaterial::BuildMaterial(&properties);

    Filesystem::ToUnixPath(finalWsRelativePath);
    materialList[finalWsRelativePath] = material;
    StoreMaterial(material);
    return material;
}

std::shared_ptr<renderer::Texture> AssetManager::ImportTexture(std::string path)
{

    // Path is valid
    if (!Filesystem::IsRegularFile(path))
        return nullptr;

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(
        path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    // Image format is supported.
    if (!pixels)
        return nullptr;

    std::filesystem::path fullImagePath = path; // full path
    std::string fileName = fullImagePath.stem().string();
    std::string imageExtension = fullImagePath.extension().string();

    std::string wsRelativePath = TEXTURE_PATH + ("/" + fileName) + TEXTURE_EXTENSION;
        
    std::string finalWsFullPath = Filesystem::GetUnusedFilePath(
        workspacePath + "/" + wsRelativePath
    );
    std::string finalWsRelativePath = Filesystem::RemoveParentPath(
        finalWsFullPath, workspacePath
    );

    std::string finalFullImagePath =
        Filesystem::ChangeExtensionTo(finalWsFullPath, imageExtension);

    std::filesystem::copy(path, finalFullImagePath);
    
    renderer::TextureBuildInfo info{};
    info.maxFilter = renderer::FILTER_LINEAR;
    info.minFilter = renderer::FILTER_LINEAR;
    info.imagePath = Filesystem::RemoveParentPath(
        finalFullImagePath,
        workspacePath
    );
    info.resourcePath = finalWsRelativePath;

    std::shared_ptr<renderer::Texture> texture = 
        renderer::VulkanTexture::BuildTextureFromBuffer(
            pixels, texWidth, texHeight, &info);

    stbi_image_free(pixels);
    Filesystem::ToUnixPath(finalWsRelativePath);
    textureList[finalWsRelativePath] = texture;
    StoreTexture(texture);
    return texture;
}

Entity* AssetManager::ImportModelObj(std::string path, Scene* scene)
{

    // Path is valid
    if (!Filesystem::IsRegularFile(path))
        return nullptr;

    renderer::BuildMeshInfo info{};
    bool result = ObjLoader2(path, info.vertices, info.indices);
    if (!result)
        return nullptr;

    std::filesystem::path fsPath = path;
    std::string fileName = fsPath.stem().string();

    std::string validWsFullPath = Filesystem::GetUnusedFilePath(
        workspacePath + "/" + MESH_PATH + "/" + fileName + MESH_DATA_EXTENSION
    );
    std::string validWsRelativePath = Filesystem::RemoveParentPath(
        validWsFullPath, workspacePath
    );

    // Store the imported mesh into workspace
    info.resourcePath =
        Filesystem::ChangeExtensionTo(validWsRelativePath, MESH_EXTENSION);
    MeshFile::Store(
        validWsFullPath,
        info.indices, info.vertices
    );

    std::shared_ptr<renderer::Mesh> mesh = renderer::VulkanMesh::BuildMesh(info);
    Filesystem::ToUnixPath(validWsRelativePath);
    meshList[validWsRelativePath] = mesh;

    // Import the mesh into the scene as entity
    Entity* entity = scene->NewEntity();
    renderer::MeshComponent* meshComp = 
        (renderer::MeshComponent*)entity->AddComponent(Component::Type::Mesh);
    meshComp->mesh = std::dynamic_pointer_cast<renderer::VulkanMesh>(mesh);

    StoreMesh(mesh);
    return entity;
}

void AssetManager::SaveToFilesystem()
{
    for(auto& e: materialList)
        StoreMaterial(e.second);
    
    for(auto& e: meshList)
        StoreMesh(e.second);

    for(auto& e: textureList)
        StoreTexture(e.second);
}

void AssetManager::DestroyResources()
{
    materialList.clear();
    meshList.clear();
    textureList.clear();
}

std::shared_ptr<renderer::Material> AssetManager::GetMaterial(std::string path)
{
    if (path == DEFAULT_MATERIAL_PATH)
    {
        return renderer::VulkanMaterial::GetDefaultMaterial();
    }

    Filesystem::ToUnixPath(path);
    std::shared_ptr<renderer::Material> material = materialList[path];
    ASSERT(material != nullptr);
    return material;
}

std::shared_ptr<renderer::Mesh> AssetManager::GetMesh(std::string path)
{
    Filesystem::ToUnixPath(path);
    std::shared_ptr<renderer::Mesh> mesh = meshList[path];
    ASSERT(mesh != nullptr);
    return mesh;
}

std::shared_ptr<renderer::Texture> AssetManager::GetTexture(std::string path)
{
    if (path == DEFAULT_TEXTURE_PATH)
    {
        return renderer::VulkanTexture::GetDefaultTexture();
    }

    Filesystem::ToUnixPath(path);
    std::shared_ptr<renderer::Texture> texture = textureList[path];
    ASSERT(texture != nullptr);
    return texture;
}

std::shared_ptr<renderer::Material> AssetManager::LoadMaterial(
    std::filesystem::path path)
{
    ASSERT(path.extension() == MATERIAL_EXTENSION);
    ASSERT(path.is_absolute());

    Json::Value json;
    std::ifstream jsonIn;
    jsonIn.open(path);
    jsonIn >> json;
    jsonIn.close();

    ASSERT(json[JSON_TYPE].asInt() == (int)JsonType::Material);
    renderer::MaterialProperties properties{};
    properties.resourcePath = Filesystem::RemoveParentPath(
        path.string(), workspacePath
    );

    DeserializeVec3(properties.albedo, json["albedo"]);
    properties.metallic = json["metallic"].asFloat();
    properties.roughness = json["roughness"].asFloat();

    std::string texturePath; 
    if((texturePath = json["albedoTexture"].asString()) != "none")
    {
        properties.albedoTexture = GetTexture(texturePath);
    }

    if((texturePath = json["metallicTexture"].asString()) != "none")
    {
        properties.metallicTexture = GetTexture(texturePath);
    }

    if((texturePath = json["roughnessTexture"].asString()) != "none")
    {
        properties.roughnessTexture = GetTexture(texturePath);
    }

    if((texturePath = json["normalTexture"].asString()) != "none")
    {
        properties.normalTexture = GetTexture(texturePath);
    }

    return renderer::VulkanMaterial::BuildMaterial(&properties);
}

bool AssetManager::StoreMaterial(std::shared_ptr<renderer::Material> material)
{

    Json::Value json;
    material->Serialize(json);
    ASSERT(json[JSON_TYPE].asInt() == (int)JsonType::Material);
    
    std::ofstream jsonOut;
    jsonOut.open(workspacePath + "/" + material->GetProperties()->resourcePath);
    jsonOut << json;
    jsonOut.close();

    return true;
}

std::shared_ptr<renderer::Texture> AssetManager::LoadTexture(
    std::filesystem::path path)
{

    ASSERT(path.extension() == TEXTURE_EXTENSION);
    ASSERT(path.is_absolute());

    Json::Value json;
    std::ifstream jsonIn;
    jsonIn.open(path);
    jsonIn >> json;
    jsonIn.close();

    ASSERT(json[JSON_TYPE].asInt() == (int)JsonType::Texture);

    renderer::TextureBuildInfo info{};
    info.addressMode = (renderer::TextureAddressMode)json["addressMode"].asInt();
    info.minFilter = (renderer::TextureFilter)json["minFilter"].asInt();
    info.maxFilter = (renderer::TextureFilter)json["maxFilter"].asInt();
    info.imagePath = json["imagePath"].asString();
    info.resourcePath = json["resourcePath"].asString();

    int texWidth, texHeight, texChannels;
    std::string fullImagePath = workspacePath + "/" + info.imagePath;
    ASSERT(path == Filesystem::ChangeExtensionTo(fullImagePath, TEXTURE_EXTENSION));

    stbi_uc* pixels = stbi_load(
        fullImagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    ASSERT(pixels != nullptr);

    std::shared_ptr<renderer::Texture> texture = 
        renderer::VulkanTexture::BuildTextureFromBuffer(
            pixels, texWidth, texHeight, &info);

    stbi_image_free(pixels);
    std::string resourcePath =
        Filesystem::ChangeExtensionTo(info.imagePath, TEXTURE_EXTENSION);
    Filesystem::ToUnixPath(resourcePath);
    textureList[resourcePath] = texture;
    return texture;
}

bool AssetManager::StoreTexture(
    std::shared_ptr<renderer::Texture> texture)
{

    Json::Value json;
    texture->Serialize(json);
    ASSERT(json[JSON_TYPE].asInt() == (int)JsonType::Texture);

    std::string resourcePath = texture->GetBuildInfo().resourcePath;
    
    std::ofstream jsonOut;
    jsonOut.open(workspacePath + "/" + resourcePath);
    jsonOut << json;
    jsonOut.close();

    return true;
}

void AssetManager::GetAvailableMeshes(std::vector<const char*>& meshPaths)
{
    meshPaths.clear();
    for (auto& p: meshList)
    {
        meshPaths.push_back(p.first.c_str());
    }
    return;
}

void AssetManager::GetAvailableMaterials(
    std::vector<const char*>& materialPaths)
{
    materialPaths.clear();
    for (auto& p: materialList)
    {
        materialPaths.push_back(p.first.c_str());
    }
    return;
}

void AssetManager::GetAvailableTextures(
    std::vector<const char*>& texturePaths)
{
    texturePaths.clear();
    for (auto& p: textureList)
    {
        texturePaths.push_back(p.first.c_str());
    }
    return;
}

void AssetManager::GetAvailableScenes(
    std::vector<std::string>& scenePaths)
{
    scenePaths.clear();

    std::string path = GetWorkspacePath();
    path = path + "/" + SCENE_PATH;
    std::vector<std::filesystem::path> entries;
    Filesystem::GetDirectoryEntries(path, entries);

    for (auto& e: entries)
    {
        std::string path = Filesystem::RemoveParentPath(
            e.string(),
            GetWorkspacePath()
        );
        scenePaths.push_back(path);
    }
}

std::string AssetManager::GetScenePath(std::string sceneName)
{
    std::string path = GetWorkspacePath();
    path = path + "/" + SCENE_PATH + "/" + sceneName + SCENE_EXTENSION;
    return path;
}

std::shared_ptr<renderer::Mesh> AssetManager::LoadMesh(
    std::filesystem::path path)
{
    ASSERT(path.extension() == MESH_EXTENSION);
    ASSERT(path.is_absolute());

    Json::Value json;
    std::ifstream jsonIn;
    jsonIn.open(path);
    jsonIn >> json;
    jsonIn.close();

    ASSERT(json[JSON_TYPE].asInt() == (int)JsonType::Mesh);
    renderer::BuildMeshInfo info{};
    info.resourcePath = json["resourcePath"].asString();

    MeshFile::Load(workspacePath + "/" +
        Filesystem::ChangeExtensionTo(info.resourcePath, MESH_DATA_EXTENSION),
        info.indices, info.vertices
    );

    std::shared_ptr<renderer::Mesh> mesh =
        renderer::VulkanMesh::BuildMesh(info);
    
    std::string materialPath = json["material"].asString();

    if (materialPath == "none")
        return mesh;
    
    std::shared_ptr<renderer::Material> material = GetMaterial(materialPath);
    mesh->AddMaterial(material);
    return mesh;
}

bool AssetManager::StoreMesh(std::shared_ptr<renderer::Mesh> mesh)
{

    Json::Value json;
    mesh->Serialize(json);
    ASSERT(json[JSON_TYPE].asInt() == (int)JsonType::Mesh);

    std::string resourcePath = json["resourcePath"].asString();
    
    std::ofstream jsonOut;
    jsonOut.open(workspacePath + "/" + resourcePath);
    jsonOut << json;
    jsonOut.close();

    return false;
}


void MeshFile::Store(
    std::string fullPath,
    const std::vector<unsigned int>& modelIndices,
    const std::vector<renderer::Vertex>& modelVertices)
{
    MeshFile header;
    header.indexSize = modelIndices.size();
    header.vertexSize = modelVertices.size();

    unsigned int indexBytes = modelIndices.size() * sizeof(unsigned int);
    unsigned int vertexBytes = modelVertices.size() * sizeof(renderer::Vertex);

    std::ofstream out;
    out.open(fullPath, std::ifstream::out | std::ifstream::binary);
    out.write((char*)&header, sizeof(header));
    out.write((char*)modelIndices.data(), indexBytes);
    out.write((char*)modelVertices.data(), vertexBytes);

    out.close();
}

void MeshFile::Load(std::string fullPath,
    std::vector<unsigned int>& modelIndices,
    std::vector<renderer::Vertex>& modelVertices)
{
    std::ifstream in;
    in.open(fullPath, std::ifstream::in | std::ifstream::binary);

    MeshFile header;
    in.read((char*)&header, sizeof(header));

    Logger::Write(
        "Loading model from workspace with mesh size " +
        std::to_string(header.vertexSize) + " bytes and index size "  +
        std::to_string(header.indexSize) + "bytes.",
        Logger::Level::Info, Logger::MsgType::Platform
    );

    modelIndices.resize(header.indexSize);
    modelVertices.resize(header.vertexSize);

    unsigned int indexBytes = modelIndices.size() * sizeof(unsigned int);
    unsigned int vertexBytes = modelVertices.size() * sizeof(renderer::Vertex);

    in.read((char*)modelIndices.data(), indexBytes);
    in.read((char*)modelVertices.data(), vertexBytes);
    in.close();
}