#pragma once

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

namespace renderer
{

struct CameraProperties
{
    bool       UseFrameExtent = true;
    glm::uvec2 Extent         = {1920, 1080};
    float      Fov            = 45.0f;
    float      ZNear          = 0.1f;
    float      ZFar           = 100.0f;
};

class Camera
{

public:
    virtual const CameraProperties& GetCamProperties() = 0;
    virtual void SetCamProperties(CameraProperties&) = 0;

    Camera() = default;
    virtual ~Camera() = default;

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
};

} // namespace renderer