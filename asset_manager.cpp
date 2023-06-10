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

void AssetManager::InitializeWorkspace(std::string path)
{
    if (!Configuration::Get(CONFIG_WORKSPACE_PATH, workspacePath))
    {
        Logger::Write(
            "Workspace path is not specified.",
            Logger::Level::Error, Logger::MsgType::Platform
        );
    }

    if (!Filesystem::IsAbsolute(workspacePath))
    {
        Logger::Write(
            "Workspace path format is not supported.",
            Logger::Level::Error, Logger::MsgType::Platform
        );
    }

    if (Filesystem::IsDirectory(workspacePath))
        LoadWorkspace();
    else
        CreateWorkspace();
}

void AssetManager::CreateWorkspace()
{
    Filesystem::CreateDirectory(workspacePath + "/" + TEXTURE_PATH);
    Filesystem::CreateDirectory(workspacePath + "/" + MESH_PATH);
    Filesystem::CreateDirectory(workspacePath + "/" + MATERIAL_PATH);
    Filesystem::CreateDirectory(workspacePath + "/" + MODEL_PATH);

    //TODO: copy default assets??
}

void AssetManager::LoadWorkspace()
{
    std::vector<std::filesystem::path> entries;

    Filesystem::GetDirectoryEntries(workspacePath + "/" + TEXTURE_PATH, entries);
    for (auto& path: entries)
    {
        if (path.extension().string() == TEXTURE_EXTENSION)
            LoadTexture(path);
    }

    Filesystem::GetDirectoryEntries(workspacePath + "/" + MATERIAL_EXTENSION, entries);
    for (auto& path: entries)
    {
        if (path.extension().string() == MATERIAL_EXTENSION)
            LoadTexture(path);
    }

    Filesystem::GetDirectoryEntries(workspacePath + "/" + MESH_EXTENSION, entries);
    for (auto& path: entries)
    {
        if (path.extension().string() == MESH_EXTENSION)
            LoadTexture(path);
    }
}

std::shared_ptr<renderer::Material> AssetManager::NewMaterial()
{
    renderer::MaterialProperties properties{};

    std::string path = TEXTURE_PATH;
    path = path + "/" + "materaial_" +
        std::to_string(materialCounter++) + MATERIAL_EXTENSION;

    properties.resourcePath = path;

    std::shared_ptr<renderer::Material> material =
        renderer::VulkanMaterial::BuildMaterial(&properties);

    materialList[path] = material;
    return material;
}

std::shared_ptr<renderer::Texture> AssetManager::ImportTexture(std::string path)
{
    {
        std::string path;
        Configuration::Get(CONFIG_WORKSPACE_PATH, path);
        ASSERT(workspacePath == path);
    }

    // Path is valid
    if (!Filesystem::IsRegularFile(path))
        return nullptr;

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(
        path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    // Image format is supported.
    if (!pixels)
        return nullptr;

    std::filesystem::path imagePath = path; // full path
    std::string extension = imagePath.extension().string();
    std::string stem = imagePath.stem().string();

    std::string wsImagePath = TEXTURE_PATH; // relative path
    wsImagePath = wsImagePath + "/" + stem + extension;
    if (Filesystem::IsRegularFile(workspacePath + "/" + wsImagePath))
    {
        int fileCount = 1;
        do {
            wsImagePath = TEXTURE_PATH;
            wsImagePath = wsImagePath + "/" +
                stem + std::to_string(fileCount++) + extension;
        } while (Filesystem::IsRegularFile(workspacePath + "/" + wsImagePath));
    }

    std::filesystem::copy(imagePath, workspacePath + "/" + wsImagePath);
    
    renderer::TextureBuildInfo info{};
    info.maxFilter = renderer::FILTER_LINEAR;
    info.minFilter = renderer::FILTER_LINEAR;
    info.imagePath = wsImagePath; // relative path
    info.resourcePath = Filesystem::ChangeExtensionTo(wsImagePath, TEXTURE_EXTENSION);

    std::shared_ptr<renderer::Texture> texture = 
        renderer::VulkanTexture::BuildTextureFromBuffer(
            pixels, texWidth, texHeight, &info);

    stbi_image_free(pixels);
    std::string resourcePath = Filesystem::ChangeExtensionTo(info.imagePath, TEXTURE_EXTENSION);
    textureList[resourcePath] = texture; // relative path
    return texture;
}


bool AssetManager::ImportModelObj(std::string path, Scene* scene)
{
    {
        std::string path;
        Configuration::Get(CONFIG_WORKSPACE_PATH, path);
        ASSERT(workspacePath == path);
    }

    // Path is valid
    if (!Filesystem::IsRegularFile(path))
        return false;

    renderer::BuildMeshInfo info{};
    bool result = ObjLoader2(path, info.vertices, info.indices);
    if (!result)
        return false;

    std::filesystem::path fsPath = path;
    std::string fileName = fsPath.stem().string();

    std::string validWsFullPath = Filesystem::GetUnusedFilePath(
        workspacePath + "/" + MESH_PATH + "/" + fileName + MESH_DATA_EXTENSION
    );
    std::string validWsRelativePath = Filesystem::RemoveParentPath(
        validWsFullPath, workspacePath
    );

    // Store the imported mesh into workspace
    info.resourcePath = Filesystem::ChangeExtensionTo(validWsRelativePath, MESH_EXTENSION);
    MeshFile::Store(
        validWsFullPath,
        info.indices, info.vertices
    );

    std::shared_ptr<renderer::Mesh> mesh = renderer::VulkanMesh::BuildMesh(info);
    meshList[validWsRelativePath] = mesh;

    // Import the mesh into the scene as entity
    Entity* entity = scene->NewEntity();
    renderer::MeshComponent* meshComp = 
        (renderer::MeshComponent*)entity->AddComponent(Component::Type::Mesh);
    meshComp->mesh = std::dynamic_pointer_cast<renderer::VulkanMesh>(mesh);

    // Store the model into the workspace
    Json::Value json;
    json[JSON_TYPE] = (int)JsonType::ObjModel;
    entity->Serialize(json["entity"]);

    std::ofstream jsonOut;
    jsonOut.open(workspacePath + "/" + validWsRelativePath);
    jsonOut << json;
    jsonOut.close();

    return true;
}

std::shared_ptr<renderer::Material> AssetManager::GetMaterial(std::string path)
{
    return materialList[path];
}

std::shared_ptr<renderer::Mesh> AssetManager::GetMesh(std::string path)
{
    return meshList[path];
}

std::shared_ptr<renderer::Texture> AssetManager::GetTexture(std::string path)
{
    return textureList[path];
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

    DeserializeVec3(properties.albedo, json["albedo"]);
    properties.metallic = json["metallic"].asFloat();
    properties.roughness = json["roughness"].asFloat();

    std::string texturePath; 
    if((texturePath = json["albedoTexture"].asString()) != "none")
    {
        properties.albedoTexture = textureList[texturePath];
    }

    if((texturePath = json["metallicTexture"].asString()) != "none")
    {
        properties.metallicTexture = textureList[texturePath];
    }

    if((texturePath = json["roughnessTexture"].asString()) != "none")
    {
        properties.roughnessTexture = textureList[texturePath];
    }

    if((texturePath = json["normalTexture"].asString()) != "none")
    {
        properties.normalTexture = textureList[texturePath];
    }

    return renderer::VulkanMaterial::BuildMaterial(&properties);
}

bool AssetManager::StoreMaterial(std::shared_ptr<renderer::Material> material)
{
    {
        std::string path;
        Configuration::Get(CONFIG_WORKSPACE_PATH, path);
        ASSERT(workspacePath == path);
    }

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
    {
        std::string path;
        Configuration::Get(CONFIG_WORKSPACE_PATH, path);
        ASSERT(workspacePath == path);
    }

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
    std::string resourcePath = Filesystem::ChangeExtensionTo(info.imagePath, TEXTURE_EXTENSION);
    textureList[resourcePath] = texture;
    return texture;
}

bool AssetManager::StoreTexture(
    std::shared_ptr<renderer::Texture> texture)
{
    {
        std::string path;
        Configuration::Get(CONFIG_WORKSPACE_PATH, path);
        ASSERT(workspacePath == path);
    }

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
    {
        std::string path;
        Configuration::Get(CONFIG_WORKSPACE_PATH, path);
        ASSERT(workspacePath == path);
    }

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
    header.indexSize = modelIndices.size() * sizeof(unsigned int);
    header.vertexSize = modelVertices.size() * sizeof(renderer::Vertex);

    std::ofstream out;
    out.open(fullPath, std::ifstream::binary);
    out.write((char*)&header, sizeof(header));
    out.write((char*)modelIndices.data(), header.indexSize);
    out.write((char*)modelVertices.data(), header.vertexSize);

    out.close();
}

void MeshFile::Load(std::string fullPath,
    std::vector<unsigned int>& modelIndices,
    std::vector<renderer::Vertex>& modelVertices)
{
    std::ifstream in;
    in.open(fullPath);

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

    in.read((char*)modelIndices.data(), header.indexSize);
    in.read((char*)modelVertices.data(), header.vertexSize);
    in.close();
}