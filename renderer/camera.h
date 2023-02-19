#pragma once

#include <glm/vec2.hpp>

namespace renderer
{

struct CameraBuildInfo
{
    bool IsPerspective = true;
    int Background = 0;
    int placeholdes = 0;
};

class Camera
{

public:
    virtual glm::vec2 GetViewSize() = 0;
    virtual void SetViewSize(glm::vec2) = 0;

};

} // namespace renderer