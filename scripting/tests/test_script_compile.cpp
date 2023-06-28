#include "scripting_subsystem.h"

#include <iostream>
#include <string>

int main(int, char**)
{
    scripting::ScriptingSystem* scriptingSystem =
        scripting::ScriptingSystem::GetInstance();
    
    scriptingSystem->Initialize();

    scripting::ScriptContext* context =
        scriptingSystem->NewContext();

    std::string source;
    while(std::getline(std::cin, source))
    {
        if (source[0] == '@')
        {
            context->ExecuteFromPath(source.substr(1));
        }
        else
        {
            context->Execute(source);
        }
    }
    
}