#pragma once

#include "vulkan_texture.h"
#include "asset_manager.h"

class TextureEditor
{

public:
    void Draw();

    TextureEditor();
    ~TextureEditor();

private:

    void ShowContent();
    void ShowTextureSelection();
    void ShowTexturePropertiesSection();
    void ShowTexturePreviewSection();

    void PublishTextureSelectedEvent(renderer::Texture* tex);

private:
    int subscriberHandle = -1;
    VkDescriptorSet imageDescSet = VK_NULL_HANDLE;

    renderer::VulkanTexture* selectedTexture = nullptr;
    bool availableTexureCached = false;
    std::vector<const char *> availableTextures;

    AssetManager* assetManager = nullptr;
};