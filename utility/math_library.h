#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/quaternion.hpp>

namespace math
{

void XrProjectionFov(glm::mat4& result, glm::vec4 fov, float nearZ, float farZ);
void XrToTransform(glm::mat4& result, glm::vec4* quaternion, glm::vec3* position);
} // namespace math
