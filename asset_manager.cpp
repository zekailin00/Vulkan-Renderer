#include "asset_manager.h"

#include "configuration.h"
#include "serialization.h"
#include "validation.h"
#include "filesystem.h"
#include "logger.h"

#include <string>

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

std::shared_ptr<renderer::Material> AssetManager::LoadMaterial(Json::Value& json)
{
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
}

