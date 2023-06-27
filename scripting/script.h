#pragma once

#include <string>

class ScriptingSubsystem;

namespace scripting
{

class Script
{

public:
    Script(const Script&) = delete;
    void operator=(const Script&) = delete;

    ~Script();

private:
    Script(std::string fileName);


    friend ScriptingSubsystem;

private:
    std::string resoucePath();
};

} // namespace scripting