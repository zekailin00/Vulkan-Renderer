#pragma once

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

namespace renderer
{

struct CameraProperties
{
    bool       UseFrameExtent = true;
    glm::uvec2 Extent         = {3840, 2160};
    float      Fov            = 45.0f; // Y-axis
    float      ZNear          = 0.01f;
    float      ZFar           = 100.0f;
};

enum class CameraType
{
    CAMERA,
    VR_DISPLAY,
    PHYSICAL_CAMERA
};

class BaseCamera
{
public:
    CameraType cameraType;
    virtual ~BaseCamera() = default;
};

class Camera: public BaseCamera
{

public:
    virtual const CameraProperties& GetCamProperties() = 0;
    virtual void SetCamProperties(CameraProperties&) = 0;

    Camera() = default;
    virtual ~Camera() = default;

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
};

class VrDisplay: public BaseCamera
{
public:
    VrDisplay() = default;
    virtual ~VrDisplay() = default;

    VrDisplay(const VrDisplay&) = delete;
    VrDisplay& operator=(const VrDisplay&) = delete;
};

} // namespace renderer