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

class Node;

class VulkanWireframe: public Wireframe
{
    std::unique_ptr<Node> GetLine(
        glm::vec3 direction, float length,
        glm::vec3 color = {0.0, 1.0, 0.0},
        float width = 1.0, bool depthEnabled = true) override;
    std::unique_ptr<Node> GetLine(
        glm::vec3 beginPoint, glm::vec3 endPoint,
        glm::vec3 color = {0.0, 1.0, 0.0},
        float width = 1.0, bool depthEnabled = true) override;
    std::unique_ptr<Node> GetSphere(
        glm::vec3 position, float radius,
        glm::vec3 color = {0.0, 1.0, 0.0},
        float width = 1.0, bool depthEnabled = true) override;
    std::unique_ptr<Node> GetCircle(
        glm::vec3 position, glm::vec3 normal, float radius,
        glm::vec3 color = {0.0, 1.0, 0.0},
        float width = 1.0, bool depthEnabled = true) override;
    std::unique_ptr<Node> GetAABB(
        glm::vec3 minCoordinates, glm::vec3 maxCoordinates,
        glm::vec3 color = {0.0, 1.0, 0.0},
        float width = 1.0, bool depthEnabled = true) override;
    std::unique_ptr<Node> GetOBB(
        glm::mat4 transform,
        glm::vec3 color = {0.0, 1.0, 0.0},
        float width = 1.0, bool depthEnabled = true) override;
};

} // namespace renderer
