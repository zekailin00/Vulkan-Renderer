#include "configuration.h"

bool Configuration::Get(std::string key, std::string& value)
{
    std::map<std::string, std::string>::iterator it;

    if ((it = configList.find(key)) == configList.end())
    {
        return false;
    }

    value = it->second;
    return true;
}

void Configuration::Set(std::string key, std::string value)
{
    configList[key] = value;
}

std::map<std::string, std::string> Configuration::configList{};