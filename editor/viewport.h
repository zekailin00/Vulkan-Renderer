#pragma once

#include "vulkan_texture.h"
#include "scene.h"

#include <imgui.h>
#include <memory>
#include <string>
#include <set>


class Viewport
{
public:
    Viewport();
    ~Viewport();

    void Draw(ImGuiID dockID);
    void DrawNoCamera(ImGuiID& nextWindowDockID);

private:
    int subscriberHandle = -1;
    VkDescriptorSet launcherDescSet;
    std::shared_ptr<renderer::VulkanTexture> launcherTexture;

    Scene* scene = nullptr;
    std::set<std::string> cameraDocking{};
};