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
    void RemoveComponent(Component::Type type);

    void ShowLightComponent();
    void ShowCameraComponent();
    void ShowVrDisplayComponent();
    void ShowMeshComponent();
    void ShowWireframeComponent();

    void PublishMaterialSelectedEvent(renderer::VulkanMaterial* mat);

private:
    int subscriberHandle = -1;

    Entity* selectedEntity = nullptr;

    bool availableMeshCached = false;
    std::vector<const char*> availableMeshes;

    bool availableMaterialCached = false;
    std::vector<const char*> availableMaterials;

    AssetManager* assetManager;
};