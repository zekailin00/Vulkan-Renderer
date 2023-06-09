#include "asset_manager.h"

#include "configuration.h"
#include "serialization.h"
#include "validation.h"
#include "filesystem.h"
#include "logger.h"

#include "obj_loader.h"

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

    // copy default assets??
}

void AssetManager::LoadWorkspace()
{
    throw;
    //TODO:
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

    std::shared_ptr<renderer::Texture> texture = 
        renderer::VulkanTexture::BuildTextureFromBuffer(
            pixels, texWidth, texHeight, &info);

    stbi_image_free(pixels);
    std::string resourcePath = Filesystem::ChangeExtensionTo(info.imagePath, TEXTURE_EXTENSION);
    textureList[resourcePath] = texture; // relative path
    return texture;
}


bool AssetManager::ImportModelObj(std::string path)
{
    {
        std::string path;
        Configuration::Get(CONFIG_WORKSPACE_PATH, path);
        ASSERT(workspacePath == path);
    }

    // Path is valid
    if (!Filesystem::IsRegularFile(path))
        return false;

    std::filesystem::path fullModelPath = path;

    renderer::BuildMeshInfo info{};
    bool result = ObjLoader2(path, info.vertices, info.indices);
    if (!result)
        return false;

    std::string relativeModelPath = path.substr(workspacePath.size() + 1);
    std::string relativeResourcePath =
        Filesystem::ChangeExtensionTo(relativeModelPath, MESH_EXTENSION);

    info.resourcePath = relativeResourcePath;

    // if ()
    //FIXME: check file with name collision

    MeshFile::Store(
        workspacePath + "/" + relativeResourcePath,
        info.indices, info.vertices
    );

    std::shared_ptr<renderer::Mesh> mesh = renderer::VulkanMesh::BuildMesh(info);
    meshList[relativeResourcePath] = mesh;
    return true;
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

    std::string imagePath = texture->GetBuildInfo().imagePath;
    std::string resourcePath = Filesystem::ChangeExtensionTo(imagePath, TEXTURE_EXTENSION);
    
    std::ofstream jsonOut;
    jsonOut.open(workspacePath + "/" + resourcePath);
    jsonOut << json;
    jsonOut.close();

    return true;
}


void MeshFile::Store(
    std::string path,
    const std::vector<unsigned int>& modelIndices,
    const std::vector<renderer::Vertex>& modelVertices)
{
    MeshFile header;
    header.indexSize = modelIndices.size() * sizeof(unsigned int);
    header.vertexSize = modelVertices.size() * sizeof(renderer::Vertex);

    std::ofstream out;
    out.open(path, std::ifstream::binary);
    out.write((char*)&header, sizeof(header));
    out.write((char*)modelIndices.data(), header.indexSize);
    out.write((char*)modelVertices.data(), header.vertexSize);

    out.close();
}

void MeshFile::Load(std::string path,
    std::vector<unsigned int>& modelIndices,
    std::vector<renderer::Vertex>& modelVertices)
{
    std::ifstream in;
    in.open(path);

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