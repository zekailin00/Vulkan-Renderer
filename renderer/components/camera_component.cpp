#include "camera_component.h"

#include "entity.h"
#include "serialization.h" 


namespace renderer
{

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

    return component;
}

void CameraComponent::Update(Timestep ts)
{
    if (!camera)
        return;

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