#include "scene.h"

#include "entity.h"
#include "validation.h"
#include "logger.h"
#include "serialization.h"

#include <glm/mat4x4.hpp>
#include <json/json.h>
#include <filesystem>
#include <fstream>
#include <vector>


Scene* Scene::NewScene(std::string name, ICoreAssetManager* manager)
{
    Scene* scene = new Scene();

    scene->sceneName = name;
    scene->state = State::Editor;
    scene->entityCounter = 0;
    scene->rootEntity = scene->_NewEntity();
    scene->assetManager = manager;
    
    return scene;
}

Scene* Scene::LoadFromFile(
    std::string path, ICoreAssetManager* manager, State state)
{
    Json::Value json;
    std::ifstream jsonIn;

    jsonIn.open(path);
    jsonIn >> json;
    jsonIn.close();

    std::filesystem::path fsPath = path;
    std::string sceneName = fsPath.stem().string();

    Scene* scene = Scene::NewScene(sceneName, manager);
    ASSERT(json[JSON_TYPE] == (int)JsonType::Scene);
    scene->state = state;
    scene->rootEntity->Deserialize(json["rootEntity"]);

    return scene;
}

bool Scene::SaveToFile(std::string path)
{
    Json::Value json;
    json[JSON_TYPE] = (int)JsonType::Scene;
    rootEntity->Serialize(json["rootEntity"]);

    std::ofstream jsonOut;
    jsonOut.open(path);
    jsonOut << json;
    jsonOut.close();

    return true;
}

Scene* Scene::Replicate(Scene::State state)
{
    std::string path = assetManager->GetScenePath(GetSceneName());
    SaveToFile(path);
    Scene* scene = LoadFromFile(path, assetManager, state);

    return scene;
}

std::shared_ptr<SceneContext> Scene::GetSceneContext(SceneContext::Type type)
{
    return contexts[type];
}

void Scene::SetSceneContext(
    SceneContext::Type type, std::shared_ptr<SceneContext> context)
{
    contexts[type] = context;
}

Entity* Scene::NewEntity()
{
    Entity* entity = _NewEntity();
    entity->parent = rootEntity;
    rootEntity->children.push_back(entity);
    entity->children.clear();

    return entity;
}

void Scene::RemoveEntity(Entity* entity)
{
    Entity::DeferredAction action;
    action.type = Entity::DeferredAction::RemoveEntity;
    action.entity = entity;
    this->PushDeferredAction(action);
}

Scene::~Scene()
{
    std::list<Entity*> childrenCopy = rootEntity->children;
    for (Entity* e: childrenCopy)
        DeferredRemoveEntity(e);
    delete rootEntity;

    for (int i = 0; i < SceneContext::Type::CtxSize; i++)
    {
        contexts[i] = nullptr;
    }
}

Entity* Scene::GetRootEntity()
{
    return rootEntity;
}

void Scene::Update(Timestep ts)
{
    ProcessDeferredActions();

    if (contexts[SceneContext::Type::RendererCtx])
    {
        std::dynamic_pointer_cast<SceneRendererContext>(
            contexts[SceneContext::Type::RendererCtx])
            ->ClearRenderData();
    }

    rootEntity->Update(ts);

    if (contexts[SceneContext::Type::RendererCtx])
    {
        std::dynamic_pointer_cast<SceneRendererContext>(
            contexts[SceneContext::Type::RendererCtx])
            ->SubmitRenderData();
    }

    if (contexts[SceneContext::Type::PhysicsCtx])
    {
        std::dynamic_pointer_cast<ScenePhysicsContext>(
            contexts[SceneContext::Type::PhysicsCtx])
            ->Simulate(ts);
    }
}

Entity* Scene::GetEntityByName(std::string name)
{
    return rootEntity->GetChildByName(name);
}

void Scene::GetEntitiesWithComponent(
    Component::Type type, std::vector<Entity*>& list)
{
    rootEntity->ScanEntities([&list, type](Entity* e){
        if (e->HasComponent(type))
            list.push_back(e);
    });
}

Entity* Scene::_NewEntity()
{
    Entity* entity = new Entity();

    for (int i = 0; i < (int)Component::Type::Size; i++)
        entity->componentList[i] = nullptr;
    entity->scene = this;

    entity->name = "Entity " + std::to_string(entityCounter++);
    entity->globalTransform = glm::mat4(1.0f);
    entity->localTransform = glm::mat4(1.0f);
    entity->children.clear();

    return entity;
}

void Scene::DeferredRemoveEntity(Entity* entity)
{
    ASSERT(this == entity->scene);
    ASSERT(entity != nullptr);

    std::list<Entity*> childrenCopy = entity->children;
    for(Entity* e: childrenCopy)
        DeferredRemoveEntity(e);

    ASSERT(entity->children.empty());

    // Remove all components
    for (int j = 0; j < (int)Component::Type::Size; j++)
        entity->DeferredRemoveComponent((Component::Type)j);

    entity->parent->children.remove(entity);
    delete entity;
}

void Scene::PushDeferredAction(const Entity::DeferredAction& action)
{
    deferredActions.push_back(action);
}

void Scene::ProcessDeferredActions()
{
    for (auto& action: deferredActions)
    {
        switch (action.type)
        {
        case Entity::DeferredAction::RemoveEntity:
        {
            DeferredRemoveEntity(action.entity);
        }
            break;
        case Entity::DeferredAction::RemoveComponent:
        {
            action.entity->DeferredRemoveComponent(
                action.target.componentType
            );
        }
            break;
        case Entity::DeferredAction::ReparentTo:
        {
            action.entity->DeferredReparentTo(
                action.target.parent
            );
        }
            break;
        }
    }

    deferredActions.clear();
}