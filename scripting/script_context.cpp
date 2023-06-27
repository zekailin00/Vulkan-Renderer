#include "script_context.h"

#include <v8-script.h>
#include <v8-local-handle.h>
#include <v8-function.h>
#include <v8-value.h>

#include <memory>

#include "validation.h"
#include "logger.h"


namespace scripting
{

Script* ScriptContext::NewScript(std::string resourcePath)
{
    Script* script = new Script();
    script->assetManager = assetManager;
    script->isolate = isolate;
    script->scriptContext = this;

    script->LoadSource(resourcePath);

    return script;
}

Script* ScriptContext::NewScript()
{
    Script* script = new Script();
    script->assetManager = assetManager;
    script->isolate = isolate;
    script->scriptContext = this;
    
    return script;
}

} // namespace scripting
