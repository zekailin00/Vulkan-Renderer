#pragma once

#include "wireframe.h"
#include <glm/vec3.hpp>

namespace renderer
{

struct WirePushConst
{
    glm::vec3 beginPoint;
	float width;
	glm::vec3 endPoint;
	float _1;
	glm::vec3 color;
	float _2;
};

class VulkanWireframe: public Wireframe
{
    std::shared_ptr<Line> GetLine(
        glm::vec3 direction, float length,
        glm::vec3 color = {0.0, 1.0, 0.0},
        float width = 1.0, bool depthEnabled = true) override;
    std::shared_ptr<Sphere> GetSphere(
        glm::vec3 position, float radius,
        glm::vec3 color = {0.0, 1.0, 0.0}, 
        float width = 1.0, bool depthEnabled = true) override;
    std::shared_ptr<Circle> GetCircle(
        glm::vec3 position, glm::vec3 normal, float radius,
        glm::vec3 color = {0.0, 1.0, 0.0},
        float width = 1.0, bool depthEnabled = true) override;
    std::shared_ptr<AABB> GetAABB(
        glm::vec3 minCoordinates, glm::vec3 maxCoordinates,
        glm::vec3 color = {0.0, 1.0, 0.0},
        float width = 1.0, bool depthEnabled = true) override;
    std::shared_ptr<OBB> GetOBB(
        glm::mat4 transform,
        glm::vec3 color = {0.0, 1.0, 0.0},
        float width = 1.0, bool depthEnabled = true) override;
};

} // namespace renderer
