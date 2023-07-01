#pragma once

#include <string>

namespace scripting
{

class IScriptAssetManager
{
public:
    virtual bool GetSourceCode(
        std::string resoucePath, std::string& source) = 0;
    virtual bool NewSourceCode(std::string fileName) = 0;
    virtual void GetAvailableScripts(std::vector<std::string>& scriptPaths) = 0;
    virtual ~IScriptAssetManager() {};
};

} // namespace scripting