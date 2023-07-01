#pragma once

#include <string>

class ICoreAssetManager
{
public:
    virtual bool IsWorkspaceInitialized() = 0;
    virtual void GetAvailableScenes(std::vector<std::string>& scenePaths) = 0;

    virtual std::string GetScenePath(std::string sceneName) = 0;
    virtual std::string GetTexturePath(std::string textureName) = 0;
    virtual std::string GetMaterialPath(std::string materialName) = 0;
    virtual std::string GetMeshPath(std::string meshName) = 0;
    virtual std::string GetScriptPath(std::string scriptName) = 0;
    virtual std::string GetWorkspacePath() = 0;
    virtual ~ICoreAssetManager() {}
};