#pragma once

#include "viewport.h"
#include "scene_graph.h"
#include "workspace.h"
#include "entity_properties.h"
#include "material_editor.h"
#include "texture_editor.h"

#include "asset_manager.h"
#include "vulkan_texture.h"

#include <vulkan/vulkan.h>
#include <memory>


class Editor
{
public:
    void DrawUI();
    Editor(Application* app);
    ~Editor();

private:
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;

    void DrawMenu();
    void DrawLauncher();
    void DrawPopups();

    std::string GetScenePath(std::string sceneName);
    void CloseScene(bool saveToFilesystem);

private:
    Application* application;
    VkDescriptorSet launcherDescSet;
    std::shared_ptr<renderer::VulkanTexture> launcherTexture;

    //** editor **//
    AssetManager* assetManager = nullptr;
    Scene* scene = nullptr;
    int activeSceneHandle = -1;
    int subscriberHandle = -1;

    Viewport viewport;
    SceneGraph sceneGraph;
    EntityProperties entityProperties;
    Workspace workspace;
    MaterialEditor materialEditor;
    TextureEditor textureEditor;
};