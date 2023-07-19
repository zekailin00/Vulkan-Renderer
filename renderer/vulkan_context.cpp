#include "vulkan_context.h"

#include <vector>

namespace renderer
{

VulkanContext::VulkanContext(
    RenderTechnique* technique,
    std::shared_ptr<LineRenderer> debugLineRenderer):
    technique(technique), debugLineRenderer(debugLineRenderer) {}

void VulkanContext::RenderDebugLine(glm::vec3 direction, float length)
{
    LineData lineData;
    VulkanLineGenerator::GetLine(lineData, direction, length);
    debugLineRenderer->AddLine(lineData);
}

void VulkanContext::RenderDebugSphere(glm::vec3 position, float radius)
{
    std::vector<LineData> lineData;
    VulkanLineGenerator::GetSphere(lineData, position, radius);
    debugLineRenderer->AddLines(lineData);
}

void VulkanContext::RenderDebugCircle(glm::vec3 position, glm::vec3 normal, float radius)
{
    std::vector<LineData> lineData;
    VulkanLineGenerator::GetCircle(lineData, position, normal, radius);
    debugLineRenderer->AddLines(lineData);
}

void VulkanContext::RenderDebugCapsule(float halfHeight, float radius,
    const glm::mat4& transform)
{
    std::vector<LineData> lineData;
    VulkanLineGenerator::GetCapsule(lineData, halfHeight, radius, transform);
    debugLineRenderer->AddLines(lineData);
}

void VulkanContext::RenderDebugAABB(glm::vec3 minCoordinates, glm::vec3 maxCoordinates)
{
    std::vector<LineData> lineData;
    VulkanLineGenerator::GetAABB(lineData, minCoordinates, maxCoordinates);
    debugLineRenderer->AddLines(lineData);
}

void VulkanContext::RenderDebugOBB(glm::mat4 transform)
{
    std::vector<LineData> lineData;
    VulkanLineGenerator::GetOBB(lineData, transform);
    debugLineRenderer->AddLines(lineData);
}

void VulkanContext::ClearRenderData()
{
    debugLineRenderer->ClearAllLines();
}

void VulkanContext::SubmitRenderData()
{
    technique->PushRendererData(debugLineRenderer);
}

} // namespace renderer