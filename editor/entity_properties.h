#pragma once

#include "application.h"

class EntityProperties
{

public:
    void Draw();

    EntityProperties();
    ~EntityProperties();

private:
    void ShowEntityProperties();
    void AddComponent();

    Entity* selectedEntity = nullptr;
    int subscriberHandle = -1;

    bool availableMeshCached = false;
    std::vector<const char*> availableMeshes;

    bool availableMaterialCached = false;
    std::vector<const char*> availableMaterials;
};