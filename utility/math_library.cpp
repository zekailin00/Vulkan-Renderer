#include "math_library.h"


namespace math
{

// Creates a projection matrix based on the specified FOV.
void XrProjectionFov(glm::mat4& result, glm::vec4 fov, float nearZ, float farZ)
{
    const float tanAngleLeft = tanf(fov[0]);
    const float tanAngleRight = tanf(fov[1]);

    const float tanAngleDown = tanf(fov[3]);
    const float tanAngleUp = tanf(fov[2]);

    const float tanAngleWidth = tanAngleRight - tanAngleLeft;
    const float tanAngleHeight = tanAngleUp - tanAngleDown;

    // Set to zero for a [0,1] Z clip space (Vulkan).
    const float offsetZ = 0;

    if (farZ <= nearZ) {
        // place the far plane at infinity
        result[0][0] = 2.0f / tanAngleWidth;
        result[1][0] = 0.0f;
        result[2][0] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
        result[3][0] = 0.0f;

        result[0][1] = 0.0f;
        result[1][1] = 2.0f / tanAngleHeight;
        result[2][1] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
        result[3][1] = 0.0f;

        result[0][2] = 0.0f;
        result[1][2] = 0.0f;
        result[2][2] = -1.0f;
        result[3][2] = -(nearZ + offsetZ);

        result[0][3] = 0.0f;
        result[1][3] = 0.0f;
        result[2][3] = -1.0f;
        result[3][3] = 0.0f;
    } else {
        // normal projection
        result[0][0] = 2.0f / tanAngleWidth;
        result[1][0] = 0.0f;
        result[2][0] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
        result[3][0] = 0.0f;

        result[0][1] = 0.0f;
        result[1][1] = 2.0f / tanAngleHeight;
        result[2][1] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
        result[3][1] = 0.0f;

        result[0][2] = 0.0f;
        result[1][2] = 0.0f;
        result[2][2] = -(farZ + offsetZ) / (farZ - nearZ);
        result[3][2] = -(farZ * (nearZ + offsetZ)) / (farZ - nearZ);

        result[0][3] = 0.0f;
        result[1][3] = 0.0f;
        result[2][3] = -1.0f;
        result[3][3] = 0.0f;
    }
}

inline void QuatToMat4(glm::mat4& result, const glm::vec4& quat) {
    const float x2 = quat[0] + quat[0];
    const float y2 = quat[1] + quat[1];
    const float z2 = quat[2] + quat[2];

    const float xx2 = quat[0] * x2;
    const float yy2 = quat[1] * y2;
    const float zz2 = quat[2] * z2;

    const float yz2 = quat[1] * z2;
    const float wx2 = quat[3] * x2;
    const float xy2 = quat[0] * y2;
    const float wz2 = quat[3] * z2;
    const float xz2 = quat[0] * z2;
    const float wy2 = quat[3] * y2;

    result[0][0] = 1.0f - yy2 - zz2;
    result[0][1] = xy2 + wz2;
    result[0][2] = xz2 - wy2;
    result[0][3] = 0.0f;

    result[1][0] = xy2 - wz2;
    result[1][1] = 1.0f - xx2 - zz2;
    result[1][2] = yz2 + wx2;
    result[1][3] = 0.0f;

    result[2][0] = xz2 + wy2;
    result[2][1] = yz2 - wx2;
    result[2][2] = 1.0f - xx2 - yy2;
    result[2][3] = 0.0f;

    result[3][0] = 0.0f;
    result[3][1] = 0.0f;
    result[3][2] = 0.0f;
    result[3][3] = 1.0f;
}

void XrToTransform(glm::mat4& result,
    glm::vec4* quaternion, glm::vec3* position)
{
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), *position);

    glm::mat4 rotation;
    QuatToMat4(rotation, *quaternion);

    result = translation * rotation;
}

} // namespace math
