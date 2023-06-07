#pragma once

#include "wireframe.h"

#include <glm/vec3.hpp>
#include <vector>


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

class VulkanWireframe
{
public:
    enum WireframeType
    {
        Line1,
        Line2,
        Sphere,
        Circle,
        AABB,
        OBB
    };
public:
    std::vector<WirePushConst> wireList;
    WireframeType type;
    glm::vec3 color;
    float width;

    virtual ~VulkanWireframe() = default;
};

class WireframeLine1: public VulkanWireframe
{
public:
    glm::vec3 direction;
    float length;
};

class WireframeLine2: public VulkanWireframe
{
public:
    glm::vec3 beginPoint;
    glm::vec3 endPoint;
};


class WireframeSphere: public VulkanWireframe
{
public:
    glm::vec3 position;
    float radius;
};

class WireframeCircle: public VulkanWireframe
{
public:
    glm::vec3 position;
    glm::vec3 normal;
    float radius;
};

class WireframeAABB: public VulkanWireframe
{
public:
    glm::vec3 minCoordinates;
    glm::vec3 maxCoordinates;
};

class WireframeOBB: public VulkanWireframe
{
public:
    glm::mat4 transform;
};


class VulkanWireframeGenerator
{
public:
    static std::shared_ptr<VulkanWireframe> GetLine(
        glm::vec3 direction, float length,
        glm::vec3 color = {0.0, 1.0, 0.0}, float width = 1.0);
    static std::shared_ptr<VulkanWireframe> GetLine(
        glm::vec3 beginPoint, glm::vec3 endPoint,
        glm::vec3 color = {0.0, 1.0, 0.0}, float width = 1.0);
    static std::shared_ptr<VulkanWireframe> GetSphere(
        glm::vec3 position, float radius,
        glm::vec3 color = {0.0, 1.0, 0.0}, float width = 1.0);
    static std::shared_ptr<VulkanWireframe> GetCircle(
        glm::vec3 position, glm::vec3 normal, float radius,
        glm::vec3 color = {0.0, 1.0, 0.0}, float width = 1.0);
    static std::shared_ptr<VulkanWireframe> GetAABB(
        glm::vec3 minCoordinates, glm::vec3 maxCoordinates,
        glm::vec3 color = {0.0, 1.0, 0.0}, float width = 1.0);
    static std::shared_ptr<VulkanWireframe> GetOBB(
        glm::mat4 transform,
        glm::vec3 color = {0.0, 1.0, 0.0}, float width = 1.0);
};

} // namespace renderer
