#pragma once

#include <string>
#include <map>

#define CONFIG_WORKSPACE_PATH "workspacePath"

class Configuration
{
public:
    static bool Get(std::string key, std::string& value);
    static void Set(std::string key, std::string value);
private:
    static std::map<std::string, std::string> configList;
};