#pragma once

#include "wireframe.h"

#include "vulkan_mesh.h"
#include "vk_primitives/vulkan_device.h"

#include <glm/gtc/constants.hpp>
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

struct LineData
{
    glm::vec3 beginPoint;
    glm::vec3 endPoint;
};

static void GetLineMesh(
    std::vector<uint32_t>& indexList, std::vector<Vertex>& vertexList,
    uint32_t resolution)
{
    uint32_t vertexCount = 0;

    const int DIVISION = resolution;
    const float DEGREE = glm::pi<float>() / DIVISION;

    for (int i = 1; i <= DIVISION; i++)
    {
        float degree1 = -glm::half_pi<float>() - DEGREE * (i - 1);
        float degree2 = -glm::half_pi<float>() - DEGREE * i;
        
        vertexList.push_back({glm::vec3(0, 0, 0), {}, {}});
        vertexList.push_back({glm::vec3(
            0.5f * glm::cos(degree1),
            0.5f * glm::sin(degree1), 0),
            {}, {}});
        vertexList.push_back({glm::vec3(
            0.5f * glm::cos(degree2),
            0.5f * glm::sin(degree2), 0), {}, {}});

        indexList.push_back(vertexCount++);
        indexList.push_back(vertexCount++);
        indexList.push_back(vertexCount++);
    }

    for (int i = 1; i <= DIVISION; i++)
    {
        float degree1 = glm::half_pi<float>() - DEGREE * (i - 1);
        float degree2 = glm::half_pi<float>() - DEGREE * i;
        
        vertexList.push_back({glm::vec3(0, 0, 0), {}, {}});
        vertexList.push_back({glm::vec3(
            0.5f * glm::cos(degree1),
            0.5f * glm::sin(degree1), 0), {}, {}});
        vertexList.push_back({glm::vec3(
            0.5f * glm::cos(degree2),
            0.5f * glm::sin(degree2), 0), {}, {}});

        indexList.push_back(vertexCount++);
        indexList.push_back(vertexCount++);
        indexList.push_back(vertexCount++);
    }

    vertexList.push_back({glm::vec3(1.0f,  0.5f, 0.0f), {}, {}});
    vertexList.push_back({glm::vec3(0.0f, -0.5f, 0.0f), {}, {}});
    vertexList.push_back({glm::vec3(0.0f,  0.5f, 0.0f), {}, {}});

    indexList.push_back(vertexCount++);
    indexList.push_back(vertexCount++);
    indexList.push_back(vertexCount++);

    vertexList.push_back({glm::vec3(0.0f, -0.5f, 0.0f), {}, {}});
    vertexList.push_back({glm::vec3(1.0f,  0.5f, 0.0f), {}, {}});
    vertexList.push_back({glm::vec3(1.0f, -0.5f, 0.0f), {}, {}});

    indexList.push_back(vertexCount++);
    indexList.push_back(vertexCount++);
    indexList.push_back(vertexCount++);
}

class VulkanLineGenerator
{
public:
    void GetLine(LineData& lineData,
        glm::vec3 direction, float length);

    void GetSphere(std::vector<LineData>& lineData,
        glm::vec3 position, float radius,
        unsigned int resolution = 64);

    void GetCircle(std::vector<LineData>& lineData,
        glm::vec3 position, glm::vec3 normal, float radius,
        unsigned int resolution = 64);

    void GetAABB(std::vector<LineData>& lineData,
        glm::vec3 minCoordinates, glm::vec3 maxCoordinates);

    void GetOBB(std::vector<LineData>& lineData,
        glm::mat4 transform);
};


class LineRenderer
{
public:
    void AddLine(LineData data);
    void AddLines(std::vector<LineData>& data);
    void ClearAllLines();

    void SetWidth(float width)
    {
        lineInfo.width = width;
    }

    float GetWidth()
    {
        return lineInfo.width;
    }
    
    VulkanBuffer<LineData>* GetLineData()
    {
        return lineInstance->GetInstanceBuffer();
    }

    LineRenderer(VulkanDevice* vulkanDevice);
    ~LineRenderer() = default;

    LineRenderer(const LineRenderer&) = delete;
    void operator=(const LineRenderer&) = delete;

private:
    struct LineInfo
    {
        float width = 0.1; /* Meter */
        bool useGlobalTransform = true;

    } lineInfo{};
    std::shared_ptr<VulkanInstanceMesh<LineData>> lineInstance = nullptr;
};

} // namespace renderer
