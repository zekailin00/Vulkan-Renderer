#include "scripting_subsystem.h"

int main(int, char**)
{
    scripting::ScriptingSystem* scriptingSystem =
        scripting::ScriptingSystem::GetInstance();
    
    scriptingSystem->Initialize();

    scripting::ScriptContext* context =
        scriptingSystem->NewContext();

    scripting::Script* script = context->NewScript();

    script->LoadSource("???", "a = 1+ 1");

}