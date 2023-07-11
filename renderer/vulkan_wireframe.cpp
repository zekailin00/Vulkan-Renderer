#include "vulkan_wireframe.h"

#include <glm/gtc/constants.hpp>
#include <tracy/Tracy.hpp>

namespace renderer
{

void VulkanLineGenerator::GetLine(LineData& lineData,
    glm::vec3 direction, float length)
{
    ZoneScopedN("VulkanLineGenerator::GetLine");

    lineData.beginPoint = -glm::normalize(direction) * (length * 0.5f);
    lineData.endPoint = glm::normalize(direction) * (length * 0.5f);
}

void VulkanLineGenerator::GetSphere(std::vector<LineData>& lineData,
    glm::vec3 position, float radius, unsigned int resolution)
{
    ZoneScopedN("VulkanLineGenerator::GetSphere");

    glm::vec3 v1 = glm::normalize(glm::vec3(1, 0, 0));
    glm::vec3 v2 = glm::normalize(glm::vec3(0.0, -v1.z, v1.y));
    glm::vec3 v3 = glm::normalize(glm::cross(v1, v2));


    const int DIVISION = resolution;
    const float DEGREE = glm::two_pi<float>() / DIVISION;


    glm::vec3 prevPoint =  radius * glm::vec3(glm::cos(0), glm::sin(0), 0) + position;
    for (int i = 1; i <= DIVISION; i++)
    {
        float currDegree = DEGREE * i;

        LineData data;
        data.beginPoint = prevPoint;
        data.endPoint = position + 
            radius * glm::vec3(glm::cos(currDegree), glm::sin(currDegree), 0);
        
        prevPoint = data.endPoint;

        lineData.push_back(data);
    }

    prevPoint = radius * glm::vec3(glm::cos(0), 0, glm::sin(0)) + position;
    for (int i = 1; i <= DIVISION; i++)
    {
        float currDegree = DEGREE * i;

        LineData data;
        data.beginPoint = prevPoint;
        data.endPoint = position + 
            radius * glm::vec3(glm::cos(currDegree), 0, glm::sin(currDegree));
        
        prevPoint = data.endPoint;

        lineData.push_back(data);
    }

    prevPoint = radius * glm::vec3(0, glm::cos(0), glm::sin(0)) + position;
    for (int i = 1; i <= DIVISION; i++)
    {
        float currDegree = DEGREE * i;

        LineData data;
        data.beginPoint = prevPoint;
        data.endPoint = position +
            radius * glm::vec3(0, glm::cos(currDegree), glm::sin(currDegree));
        
        prevPoint = data.endPoint;

        lineData.push_back(data);
    }
}

void VulkanLineGenerator::GetCircle(std::vector<LineData>& lineData,
    glm::vec3 position, glm::vec3 normal, float radius,
    unsigned int resolution)
{
    ZoneScopedN("VulkanLineGenerator::GetCircle");

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

        LineData data;
        data.beginPoint = prevPoint;
        data.endPoint = transform * radius * 
            glm::vec4(glm::cos(currDegree), glm::sin(currDegree), 0, 1);
        
        prevPoint = data.endPoint;

        lineData.push_back(data);
    }
}

void VulkanLineGenerator::GetAABB(std::vector<LineData>& lineData,
    glm::vec3 minCoordinates, glm::vec3 maxCoordinates)
{
    ZoneScopedN("VulkanLineGenerator::GetAABB");

    glm::vec3 a = minCoordinates;
    glm::vec3 b = maxCoordinates;

    LineData data;

    data.beginPoint = {a.x, a.y, a.z};
    data.endPoint   = {a.x, a.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {b.x, a.y, a.z};
    data.endPoint   = {b.x, a.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {a.x, a.y, a.z};
    data.endPoint   = {b.x, a.y, a.z};
    lineData.push_back(data);

    data.beginPoint = {a.x, a.y, b.z};
    data.endPoint   = {b.x, a.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {a.x, b.y, a.z};
    data.endPoint   = {a.x, b.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {b.x, b.y, a.z};
    data.endPoint   = {b.x, b.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {a.x, b.y, a.z};
    data.endPoint   = {b.x, b.y, a.z};
    lineData.push_back(data);

    data.beginPoint = {a.x, b.y, b.z};
    data.endPoint   = {b.x, b.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {a.x, a.y, a.z};
    data.endPoint   = {a.x, b.y, a.z};
    lineData.push_back(data);

    data.beginPoint = {b.x, a.y, b.z};
    data.endPoint   = {b.x, b.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {a.x, a.y, b.z};
    data.endPoint   = {a.x, b.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {b.x, a.y, a.z};
    data.endPoint   = {b.x, b.y, a.z};
    lineData.push_back(data);
}

void VulkanLineGenerator::GetOBB(std::vector<LineData>& lineData,
    glm::mat4 transform)
{
    ZoneScopedN("VulkanLineGenerator::GetOBB");

    glm::vec3 a = transform * glm::vec4(-0.5, -0.5, -0.5, 1);
    glm::vec3 b = transform * glm::vec4( 0.5,  0.5,  0.5, 1);

    LineData data;

    data.beginPoint = {a.x, a.y, a.z};
    data.endPoint   = {a.x, a.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {b.x, a.y, a.z};
    data.endPoint   = {b.x, a.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {a.x, a.y, a.z};
    data.endPoint   = {b.x, a.y, a.z};
    lineData.push_back(data);

    data.beginPoint = {a.x, a.y, b.z};
    data.endPoint   = {b.x, a.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {a.x, b.y, a.z};
    data.endPoint   = {a.x, b.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {b.x, b.y, a.z};
    data.endPoint   = {b.x, b.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {a.x, b.y, a.z};
    data.endPoint   = {b.x, b.y, a.z};
    lineData.push_back(data);

    data.beginPoint = {a.x, b.y, b.z};
    data.endPoint   = {b.x, b.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {a.x, a.y, a.z};
    data.endPoint   = {a.x, b.y, a.z};
    lineData.push_back(data);

    data.beginPoint = {b.x, a.y, b.z};
    data.endPoint   = {b.x, b.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {a.x, a.y, b.z};
    data.endPoint   = {a.x, b.y, b.z};
    lineData.push_back(data);

    data.beginPoint = {b.x, a.y, a.z};
    data.endPoint   = {b.x, b.y, a.z};
    lineData.push_back(data);
}

LineRenderer::LineRenderer(VulkanDevice* vulkanDevice)
{
    BuildMeshInfo info;
    info.resourcePath = "@line";

    GetLineMesh(info.indices, info.vertices, 16);

    lineInstance = VulkanInstanceMesh<LineData>::BuildMesh(
        info, vulkanDevice
    );
}

void LineRenderer::AddLine(LineData data)
{
    lineInstance->GetInstanceBuffer()->PushBack(data);
}

void LineRenderer::AddLines(std::vector<LineData>& data)
{
    VulkanBuffer<LineData>* lineDataList =
        lineInstance->GetInstanceBuffer();

    for (const LineData& e: data)
        lineDataList->PushBack(e);
}

void LineRenderer::ClearAllLines()
{
    lineInstance->GetInstanceBuffer()->Clear();
}

} // namespace renderer