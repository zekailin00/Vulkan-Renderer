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

    lineData.clear();

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

    lineData.clear();

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

void VulkanLineGenerator::GetCapsule(std::vector<LineData>& lineData,
    float halfHeight, float radius,
    const glm::mat4& transform, unsigned int resolution)
{
    ZoneScopedN("VulkanLineGenerator::GetCapsule");

    lineData.clear();

    const int DIVISION = resolution;
    const float DEGREE = glm::two_pi<float>() / DIVISION;

    glm::vec3 prevPoint = glm::vec3(
        radius * glm::cos(-glm::half_pi<float>()) + halfHeight,
        radius * glm::sin(-glm::half_pi<float>()),
        0
    );
    for (int i = 1; i <= DIVISION/2; i++)
    {
        float currDegree = DEGREE * i - glm::half_pi<float>();

        LineData data;
        data.beginPoint = prevPoint;
        data.endPoint = glm::vec3(
            radius * glm::cos(currDegree) + halfHeight,
            radius * glm::sin(currDegree),
            0);

        prevPoint = data.endPoint;

        data.beginPoint = transform * glm::vec4(data.beginPoint, 1);
        data.endPoint   = transform * glm::vec4(data.endPoint,   1);
        lineData.push_back(data);
    }

    prevPoint = glm::vec3(
        radius * glm::cos(-glm::half_pi<float>()) - halfHeight,
        radius * glm::sin(-glm::half_pi<float>()),
        0
    );
    for (int i = 1; i <= DIVISION/2; i++)
    {
        float currDegree = -DEGREE * i - glm::half_pi<float>();

        LineData data;
        data.beginPoint = prevPoint;
        data.endPoint = glm::vec3(
            radius * glm::cos(currDegree) - halfHeight,
            radius * glm::sin(currDegree),
            0);

        prevPoint = data.endPoint;

        data.beginPoint = transform * glm::vec4(data.beginPoint, 1);
        data.endPoint   = transform * glm::vec4(data.endPoint,   1);
        lineData.push_back(data);
    }

    prevPoint = glm::vec3(
        radius * glm::cos(+glm::half_pi<float>()) - halfHeight,
        0,
        radius * glm::sin(+glm::half_pi<float>())
    );
    for (int i = 1; i <= DIVISION/2; i++)
    {
        float currDegree = DEGREE * i + glm::half_pi<float>();

        LineData data;
        data.beginPoint = prevPoint;
        data.endPoint = glm::vec3(
            radius * glm::cos(currDegree) - halfHeight,
            0,
            radius * glm::sin(currDegree)
        );
        
        prevPoint = data.endPoint;

        data.beginPoint = transform * glm::vec4(data.beginPoint, 1);
        data.endPoint   = transform * glm::vec4(data.endPoint,   1);
        lineData.push_back(data);
    }

    prevPoint = glm::vec3(
        radius * glm::cos(-glm::half_pi<float>()) + halfHeight,
        0,
        radius * glm::sin(-glm::half_pi<float>()));
    for (int i = 1; i <= DIVISION/2; i++)
    {
        float currDegree = DEGREE * i - glm::half_pi<float>();

        LineData data;
        data.beginPoint = prevPoint;
        data.endPoint =  glm::vec3(
            radius * glm::cos(currDegree) + halfHeight,
            0,
            radius * glm::sin(currDegree)
        );
        
        prevPoint = data.endPoint;

        data.beginPoint = transform * glm::vec4(data.beginPoint, 1);
        data.endPoint   = transform * glm::vec4(data.endPoint,   1);
        lineData.push_back(data);
    }

    prevPoint = glm::vec3(halfHeight, radius * glm::cos(0), radius * glm::sin(0));
    for (int i = 1; i <= DIVISION; i++)
    {
        float currDegree = DEGREE * i;

        LineData data;
        data.beginPoint = prevPoint;
        data.endPoint = glm::vec3(
            halfHeight,
            radius * glm::cos(currDegree),
            radius * glm::sin(currDegree)
        );
        
        prevPoint = data.endPoint;

        data.beginPoint = transform * glm::vec4(data.beginPoint, 1);
        data.endPoint   = transform * glm::vec4(data.endPoint,   1);
        lineData.push_back(data);
    }

    prevPoint = glm::vec3(-halfHeight, radius * glm::cos(0), radius * glm::sin(0));
    for (int i = 1; i <= DIVISION; i++)
    {
        float currDegree = -DEGREE * i;

        LineData data;
        data.beginPoint = prevPoint;
        data.endPoint = glm::vec3(
            -halfHeight,
            radius * glm::cos(currDegree),
            radius * glm::sin(currDegree)
        );
        
        prevPoint = data.endPoint;

        data.beginPoint = transform * glm::vec4(data.beginPoint, 1);
        data.endPoint   = transform * glm::vec4(data.endPoint,   1);
        lineData.push_back(data);
    }

    LineData data;

    data.beginPoint = transform * glm::vec4( halfHeight, radius, 0, 1);
    data.endPoint   = transform * glm::vec4(-halfHeight, radius, 0, 1);
    lineData.push_back(data);

    data.beginPoint = transform * glm::vec4( halfHeight, -radius, 0, 1);
    data.endPoint   = transform * glm::vec4(-halfHeight, -radius, 0, 1);
    lineData.push_back(data);

    data.beginPoint = transform * glm::vec4( halfHeight, 0, radius, 1);
    data.endPoint   = transform * glm::vec4(-halfHeight, 0, radius, 1);
    lineData.push_back(data);

    data.beginPoint = transform * glm::vec4( halfHeight, 0, -radius, 1);
    data.endPoint   = transform * glm::vec4(-halfHeight, 0, -radius, 1);
    lineData.push_back(data);
}

void VulkanLineGenerator::GetAABB(std::vector<LineData>& lineData,
    glm::vec3 minCoordinates, glm::vec3 maxCoordinates)
{
    ZoneScopedN("VulkanLineGenerator::GetAABB");

    lineData.clear();

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

    lineData.clear();

    glm::vec3 a = glm::vec3(-0.5, -0.5, -0.5);
    glm::vec3 b = glm::vec3( 0.5,  0.5,  0.5);

    LineData data;

    data.beginPoint = transform * glm::vec4(a.x, a.y, a.z, 1.0f);
    data.endPoint   = transform * glm::vec4(a.x, a.y, b.z, 1.0f);
    lineData.push_back(data);

    data.beginPoint = transform * glm::vec4(b.x, a.y, a.z, 1.0f);
    data.endPoint   = transform * glm::vec4(b.x, a.y, b.z, 1.0f);
    lineData.push_back(data);

    data.beginPoint = transform * glm::vec4(a.x, a.y, a.z, 1.0f);
    data.endPoint   = transform * glm::vec4(b.x, a.y, a.z, 1.0f);
    lineData.push_back(data);

    data.beginPoint = transform * glm::vec4(a.x, a.y, b.z, 1.0f);
    data.endPoint   = transform * glm::vec4(b.x, a.y, b.z, 1.0f);
    lineData.push_back(data);

    data.beginPoint = transform * glm::vec4(a.x, b.y, a.z, 1.0f);
    data.endPoint   = transform * glm::vec4(a.x, b.y, b.z, 1.0f);
    lineData.push_back(data);

    data.beginPoint = transform * glm::vec4(b.x, b.y, a.z, 1.0f);
    data.endPoint   = transform * glm::vec4(b.x, b.y, b.z, 1.0f);
    lineData.push_back(data);

    data.beginPoint = transform * glm::vec4(a.x, b.y, a.z, 1.0f);
    data.endPoint   = transform * glm::vec4(b.x, b.y, a.z, 1.0f);
    lineData.push_back(data);

    data.beginPoint = transform * glm::vec4(a.x, b.y, b.z, 1.0f);
    data.endPoint   = transform * glm::vec4(b.x, b.y, b.z, 1.0f);
    lineData.push_back(data);

    data.beginPoint = transform * glm::vec4(a.x, a.y, a.z, 1.0f);
    data.endPoint   = transform * glm::vec4(a.x, b.y, a.z, 1.0f);
    lineData.push_back(data);

    data.beginPoint = transform * glm::vec4(b.x, a.y, b.z, 1.0f);
    data.endPoint   = transform * glm::vec4(b.x, b.y, b.z, 1.0f);
    lineData.push_back(data);

    data.beginPoint = transform * glm::vec4(a.x, a.y, b.z, 1.0f);
    data.endPoint   = transform * glm::vec4(a.x, b.y, b.z, 1.0f);
    lineData.push_back(data);

    data.beginPoint = transform * glm::vec4(b.x, a.y, a.z, 1.0f);
    data.endPoint   = transform * glm::vec4(b.x, b.y, a.z, 1.0f);
    lineData.push_back(data);
}

static void GetLineMesh(
    std::vector<uint32_t>& indexList, std::vector<Vertex>& vertexList,
    uint32_t resolution)
{
    uint32_t vertexCount = 0;

    const int DIVISION = resolution;

    for (int i = 0; i < DIVISION; i++)
    {
        float degree1 = (glm::pi<float>()) / 2 + ((i + 0) * glm::pi<float>()) / DIVISION;
        float degree2 = (glm::pi<float>()) / 2 + ((i + 1) * glm::pi<float>()) / DIVISION;
        
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

    for (int i = 0; i < DIVISION; i++)
    {
        float degree1 = (3 * glm::pi<float>()) / 2 + ((i + 0) * glm::pi<float>()) / DIVISION;
        float degree2 = (3 * glm::pi<float>()) / 2 + ((i + 1) * glm::pi<float>()) / DIVISION;
        
        vertexList.push_back({glm::vec3(0, 0, 1), {}, {0, 0}});
        vertexList.push_back({glm::vec3(
            0.5f * glm::cos(degree1) ,
            0.5f * glm::sin(degree1), 1), {}, {1, 1}});
        vertexList.push_back({glm::vec3(
            0.5f * glm::cos(degree2) ,
            0.5f * glm::sin(degree2), 1), {}, {1, 1}});

        indexList.push_back(vertexCount++);
        indexList.push_back(vertexCount++);
        indexList.push_back(vertexCount++);
    }

    vertexList.push_back({glm::vec3(0.0f, -0.5f,  0.0f), {}, {0, 0}});
    vertexList.push_back({glm::vec3(0.0f, -0.5f,  1.0f), {}, {1, 1}});
    vertexList.push_back({glm::vec3(0.0f,  0.5f,  1.0f), {}, {1, 1}});

    indexList.push_back(vertexCount++);
    indexList.push_back(vertexCount++);
    indexList.push_back(vertexCount++);

    vertexList.push_back({glm::vec3(0.0f, -0.5f, 0.0f), {}, {}});
    vertexList.push_back({glm::vec3(0.0f,  0.5f, 1.0f), {}, {}});
    vertexList.push_back({glm::vec3(0.0f,  0.5f, 0.0f), {}, {}});

    indexList.push_back(vertexCount++);
    indexList.push_back(vertexCount++);
    indexList.push_back(vertexCount++);
}

LineRenderer::LineRenderer(
        VulkanDevice* vulkanDevice,
        VulkanPipelineLayout* linePipelineLayout)
{
    BuildMeshInfo info;
    info.resourcePath = "@line";

    GetLineMesh(info.indices, info.vertices, 16);

    lineInstance = VulkanInstanceMesh<LineData>::BuildMesh(
        info, vulkanDevice
    );

    linePropUniform.Initialize(vulkanDevice, sizeof(LineRenderer::LineProperties));
    this->lineProperties = static_cast<LineProperties*>(
        linePropUniform.Map());
    
    LineProperties defaultProp{};
    *(this->lineProperties) = defaultProp;

    {
        linePipelineLayout->AllocateDescriptorSet(
            "lineProperties", 1, &linePropDescSet
        );

        std::array<VkWriteDescriptorSet, 1> descriptorWrite{};

        descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[0].dstSet = linePropDescSet;
        descriptorWrite[0].dstBinding = 0;
        descriptorWrite[0].dstArrayElement = 0;
        descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite[0].descriptorCount = 1;
        descriptorWrite[0].pBufferInfo = linePropUniform.GetDescriptor();

        vkUpdateDescriptorSets(vulkanDevice->vkDevice,
            descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
    }
}

//FIXME destructor

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