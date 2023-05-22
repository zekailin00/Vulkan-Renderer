#include "vulkan_wireframe.h"


namespace renderer
{

std::shared_ptr<Line> VulkanWireframe::GetLine(
    glm::vec3 direction, float length,
    glm::vec3 color, float width, bool depthEnabled)
{
    std::shared_ptr<Line> line = std::make_shared<Line>();

    line->color = color;
    line->width = width;
    line->depthEnabled = depthEnabled;
    line->direciton = direction;
    line->length = length;

    return line;
}

std::shared_ptr<Sphere> VulkanWireframe::GetSphere(
    glm::vec3 position, float radius,
    glm::vec3 color, float width, bool depthEnabled)
{
    std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>();

    sphere->color = color;
    sphere->width = width;
    sphere->depthEnabled = depthEnabled;
    sphere->position = position;
    sphere->radius = radius;

    return sphere;
}

std::shared_ptr<Circle> VulkanWireframe::GetCircle(
    glm::vec3 position, glm::vec3 normal, float radius,
    glm::vec3 color, float width, bool depthEnabled)
{
    std::shared_ptr<Circle> circle = std::make_shared<Circle>();

    circle->color = color;
    circle->width = width;
    circle->depthEnabled = depthEnabled;
    circle->position = position;
    circle->normal = normal;
    circle->radius = radius;

    return circle;
}

std::shared_ptr<AABB> VulkanWireframe::GetAABB(
    glm::vec3 minCoordinates, glm::vec3 maxCoordinates,
    glm::vec3 color, float width, bool depthEnabled)
{    
    std::shared_ptr<AABB> aabb = std::make_shared<AABB>();

    aabb->color = color;
    aabb->width = width;
    aabb->depthEnabled = depthEnabled;
    aabb->minCoordinates = minCoordinates;
    aabb->maxCoordinates = maxCoordinates;

    return aabb;
}

std::shared_ptr<OBB> VulkanWireframe::GetOBB(
    glm::mat4 transform,
    glm::vec3 color, float width, bool depthEnabled)
{
    std::shared_ptr<OBB> obb = std::make_shared<OBB>();

    obb->color = color;
    obb->width = width;
    obb->depthEnabled = depthEnabled;
    obb->transform = transform;

    return obb;
}

} // namespace renderer
