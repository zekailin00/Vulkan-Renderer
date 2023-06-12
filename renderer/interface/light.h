#pragma once

#include <glm/vec3.hpp>

namespace renderer
{

enum LightType
{
    SPOT_LIGHT,
    DIRECTIONAL_LIGHT,
    POINT_LIGHT
};

struct LightProperties
{
    LightType type = DIRECTIONAL_LIGHT;
    glm::uvec3 color = {10, 10, 10}; // [0, inf] intensity HDR
};

class Light
{

public:
    virtual const LightProperties& GetLightProperties() = 0;
    virtual void SetLightProperties(LightProperties&) = 0;

    Light() = default;
    virtual ~Light() = default;

    Light(const Light&) = delete;
    Light& operator=(const Light&) = delete;
};

} // namespace renderer