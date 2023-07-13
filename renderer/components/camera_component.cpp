#include "camera_component.h"

#include "entity.h"
#include "scene.h"
#include "serialization.h" 

#include "vulkan_context.h"

#include <memory>


namespace renderer
{

static void AddRenderContext(
    Entity* entity,
    RenderTechnique* technique,
    VulkanDevice* vulkanDevice,
    VulkanPipelineLayout* linePipelineLayout)
{
    Scene* scene = entity->GetScene();
    if (!scene->GetSceneContext(SceneContext::Type::RendererCtx))
    {   
        std::shared_ptr<LineRenderer> lineRenderer =
            std::make_shared<LineRenderer>(vulkanDevice, linePipelineLayout);
        LineRenderer::LineProperties* prop =
            lineRenderer->GetLineProperties();
        prop->useGlobalTransform = false;
        prop->color = {1.0f, 0.0f, 0.0f};
        prop->width = 2;

        std::shared_ptr<VulkanContext> context =
            std::make_shared<VulkanContext>(
                technique,
                lineRenderer
            );

        scene->SetSceneContext(
            SceneContext::Type::RendererCtx,
            context
        );
    }
}

Component* CameraInitializer::operator()(Entity* entity)
{
    CameraComponent* component = new CameraComponent();
    component->entity = entity;
    component->type = Component::Type::Camera;
    component->technique = technique;

    CameraProperties prop{};
    //TODO: for now, it does not support changing the parameters of the camera.
    prop.UseFrameExtent = false;
    component->camera = VulkanCamera::BuildCamera(prop);

    AddRenderContext(entity, technique, vulkanDevice, linePipelineLayout);

    return component;
}

Component* CameraDeserializer::operator()(Entity* entity, Json::Value& json)
{
    CameraComponent* component = new CameraComponent();
    component->entity = entity;
    component->type = Component::Type::Camera;
    component->technique = technique;

    CameraProperties prop{};
    prop.UseFrameExtent = false;
    component->camera = VulkanCamera::BuildCamera(prop);

    AddRenderContext(entity, technique, vulkanDevice, linePipelineLayout);

    return component;
}

void CameraComponent::Update(Timestep ts)
{
    camera->SetTransform(entity->GetGlobalTransform());
    technique->PushRendererData(camera);
}

void CameraComponent::Serialize(Json::Value& json)
{
    json["nothing"] = "nothing";
    // TODO: for now, it does not support changing parameters of the camera
}

CameraComponent::~CameraComponent()
{
    camera = nullptr;
}

} // namespace renderer