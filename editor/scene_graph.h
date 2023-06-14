#pragma once

#include "application.h"

class SceneGraph
{

public:
    void SetScene(Scene* scene);
    void Draw();

private:
    void ShowEntityChildren(const std::list<Entity*>& children, Entity** selected);
    void ShowEntityPopupContext(Entity* entity);

    void PublishEntitySelectedEvent(Entity* entity);
    void PublishNewEntityEvent(Entity* parent);
    void PublishDeleteEntityEvent(Entity* entity);

private:
    Scene* scene = nullptr;
    Entity* selectedEntity = nullptr;
};