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


} // namespace math
