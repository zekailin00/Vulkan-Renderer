#include "vulkan_wireframe.h"

#include "vulkan_node.h"
#include <glm/gtc/constants.hpp>

namespace renderer
{

std::unique_ptr<Node> VulkanWireframe::GetLine(
    glm::vec3 direction, float length,
    glm::vec3 color, float width, bool depthEnabled)
{
    std::unique_ptr<VulkanNode> node = std::make_unique<VulkanNode>();

    WirePushConst data{};
    data.beginPoint = -glm::normalize(direction) * (length * 0.5f);
    data.endPoint = glm::normalize(direction) * (length * 0.5f);
    data.color = color;
    data.width = width;

    node->wireList.push_back(data);

    return node;
}


std::unique_ptr<Node> VulkanWireframe::GetLine(
        glm::vec3 beginPoint, glm::vec3 endPoint,
        glm::vec3 color, float width, bool depthEnabled)
{
    std::unique_ptr<VulkanNode> node = std::make_unique<VulkanNode>();

    WirePushConst data{};
    data.beginPoint = beginPoint;
    data.endPoint = endPoint;
    data.color = color;
    data.width = width;

    node->wireList.push_back(data);

    return node;
}

std::unique_ptr<Node> VulkanWireframe::GetSphere(
    glm::vec3 position, float radius,
    glm::vec3 color, float width, bool depthEnabled)
{
    std::unique_ptr<VulkanNode> node = std::make_unique<VulkanNode>();

    glm::vec3 v1 = glm::normalize(glm::vec3(1, 0, 0));
    glm::vec3 v2 = glm::normalize(glm::vec3(0.0, -v1.z, v1.y));
    glm::vec3 v3 = glm::normalize(glm::cross(v1, v2));


    const int DIVISION = 32;
    const float DEGREE = glm::two_pi<float>() / DIVISION;


    glm::vec3 prevPoint =  radius * glm::vec3(glm::cos(0), glm::sin(0), 0) + position;
    for (int i = 1; i <= DIVISION; i++)
    {
        float currDegree = DEGREE * i;

        WirePushConst data{};
        data.color = color;
        data.width = width;
        data.beginPoint = prevPoint;
        data.endPoint = 
            radius * glm::vec3(glm::cos(currDegree), glm::sin(currDegree), 0) + position;
        
        prevPoint = data.endPoint;

        node->wireList.push_back(data);
    }

    prevPoint = radius * glm::vec3(glm::cos(0), 0, glm::sin(0)) + position;
    for (int i = 1; i <= DIVISION; i++)
    {
        float currDegree = DEGREE * i;

        WirePushConst data{};
        data.color = color;
        data.width = width;
        data.beginPoint = prevPoint;
        data.endPoint = 
            radius * glm::vec3(glm::cos(currDegree), 0, glm::sin(currDegree)) + position;
        
        prevPoint = data.endPoint;

        node->wireList.push_back(data);
    }

    prevPoint =  radius * glm::vec3(0, glm::cos(0), glm::sin(0)) + position;
    for (int i = 1; i <= DIVISION; i++)
    {
        float currDegree = DEGREE * i;

        WirePushConst data{};
        data.color = color;
        data.width = width;
        data.beginPoint = prevPoint;
        data.endPoint = 
            radius * glm::vec3(0, glm::cos(currDegree), glm::sin(currDegree)) + position;
        
        prevPoint = data.endPoint;

        node->wireList.push_back(data);
    }

    return node;
}

std::unique_ptr<Node> VulkanWireframe::GetCircle(
    glm::vec3 position, glm::vec3 normal, float radius,
    glm::vec3 color, float width, bool depthEnabled)
{
    std::unique_ptr<VulkanNode> node = std::make_unique<VulkanNode>();

    glm::vec3 v1 = glm::normalize(normal);
    glm::vec3 v2 = glm::normalize(glm::vec3(0.0, -v1.z, v1.y));
    glm::vec3 v3 = glm::normalize(glm::cross(v1, v2));

    glm::mat4 transform = glm::mat4(
        glm::vec4(v2, 0),
        glm::vec4(v3, 0),
        glm::vec4(v1, 0),
        glm::vec4(position, 1)
    );

    const int DIVISION = 32;
    const float DEGREE = glm::two_pi<float>() / DIVISION;
    glm::vec3 prevPoint = transform * radius * 
        glm::vec4(glm::cos(0), glm::sin(0), 0, 1);
    for (int i = 1; i <= DIVISION; i++)
    {
        float currDegree = DEGREE * i;

        WirePushConst data{};
        data.color = color;
        data.width = width;
        data.beginPoint = prevPoint;
        data.endPoint = transform * radius * 
            glm::vec4(glm::cos(currDegree), glm::sin(currDegree), 0, 1);
        
        prevPoint = data.endPoint;

        node->wireList.push_back(data);
    }

    return node;
}

std::unique_ptr<Node> VulkanWireframe::GetAABB(
    glm::vec3 minCoordinates, glm::vec3 maxCoordinates,
    glm::vec3 color, float width, bool depthEnabled)
{    
    std::unique_ptr<VulkanNode> node = std::make_unique<VulkanNode>();

    glm::vec3 a = minCoordinates;
    glm::vec3 b = maxCoordinates;

    WirePushConst data{};
    data.color = color;
    data.width = width;

    data.beginPoint = {a.x, a.y, a.z};
    data.endPoint   = {a.x, a.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {b.x, a.y, a.z};
    data.endPoint   = {b.x, a.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {a.x, a.y, a.z};
    data.endPoint   = {b.x, a.y, a.z};
    node->wireList.push_back(data);

    data.beginPoint = {a.x, a.y, b.z};
    data.endPoint   = {b.x, a.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {a.x, b.y, a.z};
    data.endPoint   = {a.x, b.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {b.x, b.y, a.z};
    data.endPoint   = {b.x, b.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {a.x, b.y, a.z};
    data.endPoint   = {b.x, b.y, a.z};
    node->wireList.push_back(data);

    data.beginPoint = {a.x, b.y, b.z};
    data.endPoint   = {b.x, b.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {a.x, a.y, a.z};
    data.endPoint   = {a.x, b.y, a.z};
    node->wireList.push_back(data);

    data.beginPoint = {b.x, a.y, b.z};
    data.endPoint   = {b.x, b.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {a.x, a.y, b.z};
    data.endPoint   = {a.x, b.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {b.x, a.y, a.z};
    data.endPoint   = {b.x, b.y, a.z};
    node->wireList.push_back(data);

    return node;
}

std::unique_ptr<Node> VulkanWireframe::GetOBB(
    glm::mat4 transform,
    glm::vec3 color, float width, bool depthEnabled)
{
    std::unique_ptr<VulkanNode> node = std::make_unique<VulkanNode>();

    glm::vec3 a = transform * glm::vec4(-0.5, -0.5, -0.5, 1);
    glm::vec3 b = transform * glm::vec4( 0.5,  0.5,  0.5, 1);

    WirePushConst data{};
    data.color = color;
    data.width = width;

    data.beginPoint = {a.x, a.y, a.z};
    data.endPoint   = {a.x, a.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {b.x, a.y, a.z};
    data.endPoint   = {b.x, a.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {a.x, a.y, a.z};
    data.endPoint   = {b.x, a.y, a.z};
    node->wireList.push_back(data);

    data.beginPoint = {a.x, a.y, b.z};
    data.endPoint   = {b.x, a.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {a.x, b.y, a.z};
    data.endPoint   = {a.x, b.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {b.x, b.y, a.z};
    data.endPoint   = {b.x, b.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {a.x, b.y, a.z};
    data.endPoint   = {b.x, b.y, a.z};
    node->wireList.push_back(data);

    data.beginPoint = {a.x, b.y, b.z};
    data.endPoint   = {b.x, b.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {a.x, a.y, a.z};
    data.endPoint   = {a.x, b.y, a.z};
    node->wireList.push_back(data);

    data.beginPoint = {b.x, a.y, b.z};
    data.endPoint   = {b.x, b.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {a.x, a.y, b.z};
    data.endPoint   = {a.x, b.y, b.z};
    node->wireList.push_back(data);

    data.beginPoint = {b.x, a.y, a.z};
    data.endPoint   = {b.x, b.y, a.z};
    node->wireList.push_back(data);

    return node;
}

} // namespace renderer
