#include "script_component.h"

#include "scene.h"
#include "entity.h"
#include "script_context.h"
#include "script_asset_manager.h"

#include "validation.h"


namespace scripting
{

Component* ScriptInitializer::operator()(Entity* entity)
{
    ScriptComponent* component = new ScriptComponent();
    component->entity = entity;
    component->type = Component::Type::Script;

    Scene* scene = entity->GetScene();

    ScriptContext* context = dynamic_cast<ScriptContext*>(
        scene->GetSceneContext(Component::Type::Script));
    IScriptAssetManager* assetManaget = 
        dynamic_cast<IScriptAssetManager*>(scene->GetAssetManager());

    component->script = context->NewScript(assetManaget);
    return component;
}

Component* ScriptDeserializer::operator()(Entity* entity, Json::Value& json)
{
    ScriptComponent* component = new ScriptComponent();
    component->entity = entity;
    component->type = Component::Type::Script;

    Scene* scene = entity->GetScene();

    ScriptContext* context = dynamic_cast<ScriptContext*>(
        scene->GetSceneContext(Component::Type::Script));
    IScriptAssetManager* assetManaget = 
        dynamic_cast<IScriptAssetManager*>(scene->GetAssetManager());

    component->script = context->NewScript(assetManaget);

    std::string resourcePath = json["resourcePath"].asString();

    if (resourcePath == "None")
    {
        return component;
    }

    bool result = component->script->LoadSource(resourcePath);
    ASSERT(result == true);
    return component;
}

void ScriptComponent::Update(Timestep ts)
{
    ASSERT(script != nullptr);

    Scene::State state = entity->GetScene()->GetState();
    
    if (state == Scene::State::Running ||
        state == Scene::State::RunningVR)
    {
        script->Run(ts);
    }
}

void ScriptComponent::Serialize(Json::Value& json)
{
    json["resourcePath"] = script->GetResourcePath();
}

ScriptComponent::~ScriptComponent()
{
    script->RunOnDestroyed();
    delete script;
}

} // namespace scripting
