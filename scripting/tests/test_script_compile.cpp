#include "scripting_subsystem.h"

int main(int, char**)
{
    scripting::ScriptingSystem* scriptingSystem =
        scripting::ScriptingSystem::GetInstance();
    
    scriptingSystem->Initialize();

    scripting::ScriptContext* context =
        scriptingSystem->NewContext();

    scripting::Script* script = context->NewScript();

    std::string sourceCode = "source";
    sourceCode = sourceCode + scripting::ScriptingSystem::scriptResource;
    sourceCode = sourceCode + "\nsystem.print(\"Hello World\")";

    script->LoadSource("scripts/source.js", sourceCode);

}