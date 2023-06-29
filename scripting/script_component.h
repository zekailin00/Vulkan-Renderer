#pragma once

#include "component.h"
#include "script.h"

namespace scripting
{

class ScriptInitializer
{

public:
    ScriptInitializer() {}

    Component* operator()(Entity* entity);
};

class ScriptDeserializer
{

public:
    ScriptDeserializer() {}

    Component* operator()(Entity* entity, Json::Value& json);
};

struct ScriptComponent: public Component
{
    Script* script;

    void Update(Timestep ts) override;
    void Serialize(Json::Value& json) override;
    ~ScriptComponent() override;

private:
    friend ScriptInitializer;
    friend ScriptDeserializer;
};

} // namespace scripting
