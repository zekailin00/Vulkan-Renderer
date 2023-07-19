#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

namespace math
{

void XrProjectionFov(glm::mat4& result, glm::vec4 fov, float nearZ, float farZ);
void XrToTransform(glm::mat4& result, glm::vec4* quaternion, glm::vec3* position);

void RotateAroundBasis0(glm::mat4& transform, float radian);
void RotateAroundBasis1(glm::mat4& transform, float radian);
void RotateAroundBasis2(glm::mat4& transform, float radian);

} // namespace math
