#pragma once

#include <v8-context.h>

namespace scripting
{

class ScriptingSubsystem;

class ScriptContext
{
public:
    ScriptContext(const ScriptContext&) = delete;
    void operator=(const ScriptContext&) = delete;

private:
    ScriptContext();
    ~ScriptContext()
    {
        context.Reset();
    }
    
    friend ScriptingSubsystem;

private:
    v8::Global<v8::Context> context;
};

} // namespace scripting