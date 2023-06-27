#pragma once

#include <string>

namespace scripting
{

class IScriptAssetManager
{
public:
    virtual bool GetSourceCode(
        std::string resoucePath, std::string& source) = 0;
    virtual ~IScriptAssetManager() {};
};

} // namespace scripting