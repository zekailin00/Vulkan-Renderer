#include "script_component.h"

#include "scene.h"
#include "entity.h"
#include "script_context.h"
#include "scripting_subsystem.h"
#include "script_asset_manager.h"

#include "validation.h"
#include "asset_manager.h"

#include <memory>


namespace scripting
{

Component* ScriptInitializer::operator()(Entity* entity)
{
    ScriptComponent* component = new ScriptComponent();
    component->entity = entity;
    component->type = Component::Type::Script;

    Scene* scene = entity->GetScene();

    std::shared_ptr<ScriptContext> context =
        std::dynamic_pointer_cast<ScriptContext>(
            scene->GetSceneContext(SceneContext::Type::ScriptCtx));

    if (context == nullptr)
    {
        ScriptingSystem* scriptingSystem = ScriptingSystem::GetInstance();
        context.reset(scriptingSystem->NewContext());
        scene->SetSceneContext(SceneContext::Type::ScriptCtx, context);
    }

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

    std::shared_ptr<ScriptContext> context =
        std::dynamic_pointer_cast<ScriptContext>(
            scene->GetSceneContext(SceneContext::Type::ScriptCtx));

    if (context == nullptr)
    {
        ScriptingSystem* scriptingSystem = ScriptingSystem::GetInstance();
        context.reset(scriptingSystem->NewContext());
        scene->SetSceneContext(SceneContext::Type::ScriptCtx, context);
    }

    IScriptAssetManager* assetManaget = 
        dynamic_cast<IScriptAssetManager*>(
            dynamic_cast<AssetManager*>(scene->GetAssetManager()));

    component->script = context->NewScript(assetManaget);

    std::string resourcePath = json["resourcePath"].asString();

    if (resourcePath == "None")
    {
        return component;
    }

    bool result = component->script->LoadSource(resourcePath, entity);
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
