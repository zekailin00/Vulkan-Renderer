#pragma once

#include "application.h"

class SceneGraph
{

public:

    SceneGraph();
    ~SceneGraph();
    void SetScene(Scene* scene);
    void Draw();

private:
    SceneGraph(const SceneGraph&) = delete;
    const SceneGraph& operator=(const SceneGraph) = delete; 

    void ShowEntityChildren(const std::list<Entity*>& children);
    void ShowEntityPopupContext(Entity* entity);

    void PublishEntitySelectedEvent(Entity* entity);
    void PublishNewEntityEvent(Entity* parent);
    void PublishDeleteEntityEvent(Entity* entity);

private:
    int subscriberHandle = -1;
    Scene* scene = nullptr;
    Entity* selectedEntity = nullptr;
};