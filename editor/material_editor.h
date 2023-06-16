#pragma once

#include "vulkan_material.h"
#include "asset_manager.h"

#include <memory>


class MaterialEditor
{

public:
    void Draw();
    
    MaterialEditor();
    ~MaterialEditor();

private:
    void ShowMaterialProperties();

    void ShowAlbedoSection(const renderer::MaterialProperties* properties);
    void ShowMetallicSection(const renderer::MaterialProperties* properties);
    void ShowRoughnessSection(const renderer::MaterialProperties* properties);

    void PublishMaterialSelectedEvent(renderer::Material* mat);
    void PublishTextureSelectedEvent(renderer::Texture* tex);

private:
    int subscriberHandle = -1;

    renderer::VulkanMaterial* selectedMat = nullptr;

    std::vector<const char*> availableMaterials;
    bool availableMaterialCached = false;

    std::vector<const char*> availableTextures;
    bool availableTexureCached = false;

    AssetManager* assetManager = nullptr;
};