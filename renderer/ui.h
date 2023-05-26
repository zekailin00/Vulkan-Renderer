#pragma once

#include "texture.h"

#include <glm/vec2.hpp>
#include <memory>

namespace renderer
{

struct UIBuildInfo
{
    glm::vec2 extent;
    void (*imgui)(void);
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