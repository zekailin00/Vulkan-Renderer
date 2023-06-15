#pragma once

#include "vulkan_texture.h"

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
    renderer::VulkanTexture* selectedTexture = nullptr;
    int subscriberHandle = -1;

    VkDescriptorSet imageDescSet = VK_NULL_HANDLE;

    bool availableTexureCached = false;
    std::vector<const char *> availableTextures;
};