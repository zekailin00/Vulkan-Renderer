#pragma once

class ICoreAssetManager
{
public:
    virtual bool IsWorkspaceInitialized() = 0;
    virtual std::string GetScenePath(std::string sceneName) = 0;
    virtual std::string GetTexturePath(std::string textureName) = 0;
    virtual std::string GetMaterialPath(std::string materialName) = 0;
    virtual std::string GetMeshPath(std::string meshName) = 0;
    virtual std::string GetWorkspacePath() = 0;
};