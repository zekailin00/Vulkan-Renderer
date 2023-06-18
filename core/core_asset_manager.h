#pragma once

class ICoreAssetManager
{
public:
    virtual bool IsWorkspaceInitialized() = 0;
    virtual std::string GetScenePath(std::string sceneName) = 0;
};