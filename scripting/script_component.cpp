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
    return nullptr;
}

void ScriptComponent::Update(Timestep ts)
{
    ASSERT(script != nullptr);

    
}

void ScriptComponent::Serialize(Json::Value& json)
{

}

ScriptComponent::~ScriptComponent()
{

}

} // namespace scripting
