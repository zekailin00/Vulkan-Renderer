#pragma once

#include "texture.h"

#include <glm/vec2.hpp>
#include <functional>
#include <memory>

namespace renderer
{

struct UIBuildInfo
{
    // When ui is rendered to glfw window,
    // extent is reset to window extent.
    glm::vec2 extent = {720, 720};

    // FIXME: not serializable for now 
    std::function<void()> imgui;

    /**
     * @brief When editorUI is set to true,
     * the ui takes glfw inputs, and renderUI commands
     * are not wrapped into a signle window.
     */
    bool editorUI = false;
};

class UI
{
public:
    UI() = default;
    virtual ~UI() = default;

    UI(const UI&) = delete;
    const UI& operator=(const UI&) = delete;

    virtual void SetExtent(glm::vec2 extent) = 0;
    virtual std::shared_ptr<Texture> GetTexture() = 0;
};

} // namespace renderer