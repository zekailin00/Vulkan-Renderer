#pragma once

#include "wireframe.h"

#include "vulkan_mesh.h"
#include "vk_primitives/vulkan_device.h"
#include "vk_primitives/vulkan_uniform.h"
#include "vk_primitives/vulkan_pipeline_layout.h"

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

class VulkanLineGenerator
{
public:
    static void GetLine(LineData& lineData,
        glm::vec3 direction, float length);

    static void GetSphere(std::vector<LineData>& lineData,
        glm::vec3 position, float radius,
        unsigned int resolution = 64);

    static void GetCircle(std::vector<LineData>& lineData,
        glm::vec3 position, glm::vec3 normal, float radius,
        unsigned int resolution = 64);

    static void GetAABB(std::vector<LineData>& lineData,
        glm::vec3 minCoordinates, glm::vec3 maxCoordinates);

    static void GetOBB(std::vector<LineData>& lineData,
        glm::mat4 transform);
};


class LineRenderer
{
public:
    struct LineProperties // Sent to GPU, has alignment requirements
    {
        glm::mat4 model;

        /* RGB float [0.0, 1.0] */
        glm::vec3 color = {1.0f, 1.0f, 1.0f};
        float _0;

        float width = 2; /* Pixel count */
        int useGlobalTransform = 1;
        glm::vec2 resolution;
    };

public:
    void AddLine(LineData data);
    void AddLines(std::vector<LineData>& data);
    void ClearAllLines();

    void SetLineWidth(float width)
    {
        lineProperties->width = width;
    }

    float GetLineWidth()
    {
        return lineProperties->width;
    }
    
    VulkanBuffer<LineData>* GetLineData()
    {
        return lineInstance->GetInstanceBuffer();
    }
    
    VkDescriptorSet* GetLinePropDescSet()
    {
        return &linePropDescSet;
    }

    LineProperties* GetLineProperties()
    {
        return lineProperties;
    }

    VkBuffer* GetVertexBuffer()
    {
        return &lineInstance->GetVertexbuffer().vertexBuffer;
    }

    VkBuffer* GetIndexBuffer()
    {
        return &lineInstance->GetVertexbuffer().indexBuffer;
    }

    VkBuffer* GetInstanceBuffer()
    {
        return lineInstance->GetInstanceBuffer()->GetBuffer();
    }

    uint32_t GetIndexCount()
    {
        return lineInstance->GetVertexbuffer().GetIndexCount();
    }

    uint32_t GetInstanceCount()
    {
        return lineInstance->GetInstanceCount();
    }

    LineRenderer(
        VulkanDevice* vulkanDevice,
        VulkanPipelineLayout* linePipelineLayout
    );
    ~LineRenderer() = default;

    LineRenderer(const LineRenderer&) = delete;
    void operator=(const LineRenderer&) = delete;

private:
    VulkanUniform linePropUniform;
    std::shared_ptr<VulkanInstanceMesh<LineData>> lineInstance = nullptr;

    LineProperties* lineProperties = nullptr;
    VkDescriptorSet linePropDescSet = VK_NULL_HANDLE;
};

} // namespace renderer
