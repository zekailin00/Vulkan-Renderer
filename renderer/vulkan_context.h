#pragma once

#include "scene_contexts.h"

#include "vulkan_wireframe.h"
#include "render_technique.h"

#include <memory>

namespace renderer
{

class VulkanRenderer;

class VulkanContext: public SceneRendererContext
{
public:
    void RenderDebugLine(glm::vec3 direction, float length) override;

    void RenderDebugSphere(glm::vec3 position, float radius) override;

    void RenderDebugCircle(glm::vec3 position, glm::vec3 normal, float radius) override;

    void RenderDebugCapsule(float halfHeight, float radius,
        const glm::mat4& transform) override;

    void RenderDebugAABB(glm::vec3 minCoordinates, glm::vec3 maxCoordinates) override;

    void RenderDebugOBB(glm::mat4 transform) override;

    void ClearRenderData() override;

    void SubmitRenderData() override;

    VulkanContext(
        RenderTechnique* technique,
        std::shared_ptr<LineRenderer> debugLineRenderer);
    ~VulkanContext() = default;

    VulkanContext(const VulkanContext&) = delete;
    void operator=(const VulkanContext&) = delete;

private:
    friend VulkanRenderer;

    RenderTechnique* technique;
    std::shared_ptr<LineRenderer> debugLineRenderer;
};

} // namespace renderer