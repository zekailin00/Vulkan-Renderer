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
    void ShowScriptComponent();
    void ShowDynamicBodyComponent();
    void ShowStaticBodyComponent();

    bool CheckComponentAddDependencies(Component::Type type);

    void PublishMaterialSelectedEvent(renderer::VulkanMaterial* mat);

private:
    int subscriberHandle = -1;

    Entity* selectedEntity = nullptr;

    bool availableMeshCached = false;
    std::vector<const char*> availableMeshes;

    bool availableMaterialCached = false;
    std::vector<const char*> availableMaterials;

    bool availableScriptCached = false;
    std::vector<std::string> availableScripts;

    AssetManager* assetManager;
};