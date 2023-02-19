#pragma once

#include <glm/vec2.hpp>

namespace renderer
{

struct CameraProperties
{
    bool      UseFrameExtent = true;
    glm::vec2 Extent         = {1920, 1080};
    float     Fov            = 45.0f;
    float     ZNear          = 0.1f;
    float     ZFar           = 100.0f;
};

class Camera
{

public:
    virtual CameraProperties* GetCamProperties() = 0;

    Camera() = default;
    virtual ~Camera() = default;

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
};

} // namespace renderer