#include "scene.h"

#include "entity.h"
#include "validation.h"
#include "logger.h"

#include <glm/mat4x4.hpp>
#include <json/json.h>
#include <fstream>


Scene* Scene::NewScene()
{
    Scene* scene = new Scene();

    scene->state = State::Editor;
    scene->entityCounter = 0;
    scene->rootEntity = scene->_NewEntity();
    
    return scene;
}

Scene* Scene::LoadFromFile(std::string path, State state)
{
    Json::Value json;
    std::ifstream jsonIn;

    jsonIn.open(path);
    jsonIn >> json;
    jsonIn.close();

    Scene* scene = Scene::NewScene();
    ASSERT(json["FileType"] == "Scene");
    scene->state = state;
    scene->rootEntity->Deserialize(json["rootEntity"]);

    return scene;
}

bool Scene::SaveToFile(std::string path)
{
    Json::Value json;
    json["FileType"] = "Scene";
    rootEntity->Serialize(json["rootEntity"]);

    std::ofstream jsonOut;
    jsonOut.open(path);
    jsonOut << json;
    jsonOut.close();

    return true;
}

Scene* Scene::Replicate(Scene::State state)
{
    SaveToFile("tmp.json");
    Scene* scene = LoadFromFile("tmp.json", state);

    return scene;
}

Entity* Scene::NewEntity()
{

    Entity* entity = _NewEntity();
    entity->parent = rootEntity;
    rootEntity->children.push_back(entity);
    entity->children.clear();

    return entity;
}

bool Scene::RemoveEntity(Entity* entity)
{
    ASSERT(this == entity->scene);
    ASSERT(entity != nullptr);

    std::list<Entity*> childrenCopy = entity->children;
    for(Entity* e: childrenCopy)
        RemoveEntity(e);

    ASSERT(entity->children.empty());

    // Remove all components
    for (int j = 0; j < (int)Component::Type::Size; j++)
        entity->RemoveComponent((Component::Type)j);

    entity->parent->children.remove(entity);
    delete entity;

    return true;
}

Scene::~Scene()
{
    std::list<Entity*> childrenCopy = rootEntity->children;
    for (Entity* e: childrenCopy)
        RemoveEntity(e);
    delete rootEntity;
}

Entity* Scene::GetRootEntity()
{
    return rootEntity;
}

void Scene::Update(Timestep ts)
{
    rootEntity->Update(ts);
}

Entity* Scene::GetEntityByName(std::string name)
{
    return rootEntity->GetChildByName(name);
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