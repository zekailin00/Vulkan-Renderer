#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace math
{

void XrProjectionFov(glm::mat4& result, glm::vec4 fov, float nearZ, float farZ);

} // namespace math
