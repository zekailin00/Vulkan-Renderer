#include "openxr_components.h"

#include "entity.h"
#include "scene.h"
#include "validation.h"

#include "input.h"
#include "math_library.h"

#include <memory>


namespace renderer
{

Component* VrDisplayInitializer::operator()(Entity* entity)
{
    VrDisplayComponent* component = new VrDisplayComponent();
    component->entity = entity;
    component->type = Component::Type::VrDisplay;
    
    component->technique = this->technique;
    component->vrDisplay = VulkanVrDisplay::BuildCamera();
    return component;
}

Component* VrDisplayDeserializer::operator()(
    Entity* entity, Json::Value& json)
{
    VrDisplayComponent* component = new VrDisplayComponent();
    component->entity = entity;
    component->type = Component::Type::VrDisplay;
    
    component->technique = this->technique;
    component->vrDisplay = VulkanVrDisplay::BuildCamera();
    return component;
}

void VrDisplayComponent::Update(Timestep ts)
{
    Scene::State state = entity->GetScene()->GetState();

    if (state == Scene::State::Editor)
    {
        std::shared_ptr<VulkanCamera> camera =
            vrDisplay->GetLeftCamera();

        ASSERT(camera != nullptr);

        camera->SetTransform(entity->GetGlobalTransform());
        technique->PushRendererData(camera);
    }
    else if (state == Scene::State::RunningVR)
    {
        std::shared_ptr<VulkanVrDisplay> vkCamera = this->vrDisplay;

        Input* input = Input::GetInstance();
        std::shared_ptr<VulkanCamera> cameraL =
            vrDisplay->GetLeftCamera();
        std::shared_ptr<VulkanCamera> cameraR =
            vrDisplay->GetRightCamera();
        
        ASSERT(cameraL != nullptr);
        ASSERT(cameraR != nullptr);

        const renderer::CameraProperties& propL = cameraL->GetCamProperties();
        const renderer::CameraProperties& propR = cameraR->GetCamProperties();

        {   // project matrices of the left eye
            glm::vec4 xrFoV = input->xr_left_eye_fov;
            cameraL->SetProjection(xrFoV, propL.ZNear, propL.ZFar);
        }

        {   // project matrices of the right eye
            glm::vec4 xrFoV = input->xr_right_eye_fov;
            cameraR->SetProjection(xrFoV, propR.ZNear, propR.ZFar);
        }

        {   // Set local transform of the entity
            const glm::vec3& xrPosL = input->xr_left_eye_pos;
            const glm::vec3& xrPosR = input->xr_right_eye_pos;
            glm::vec3 xrPosC = (xrPosL + xrPosR) / 2.0f;
            glm::mat4 translation = glm::translate(glm::mat4(1.0f), xrPosC);

            glm::vec4 xrQuat = input->xr_left_eye_quat;
            glm::quat quat;
            quat.x = xrQuat[0];
            quat.y = xrQuat[1];
            quat.z = xrQuat[2];
            quat.w = xrQuat[3];
            glm::mat4 rotation = glm::toMat4(quat);

            entity->SetLocalTransform(translation * rotation);
        }

        {   // view matrix of the left eye
            glm::vec4 xrQuat = input->xr_left_eye_quat;
            glm::vec3 xrPos = input->xr_left_eye_pos;
            glm::quat quat;
            quat.x = xrQuat[0];
            quat.y = xrQuat[1];
            quat.z = xrQuat[2];
            quat.w = xrQuat[3];

            glm::mat4 rotation = glm::toMat4(quat);
            glm::mat4 translation = glm::translate(glm::mat4(1.0f), xrPos);

            cameraL->SetTransform(
                entity->GetParent()->GetGlobalTransform() * translation * rotation);
        }

        {   // view matrix of the right eye
            glm::vec4 xrQuat = input->xr_right_eye_quat;
            glm::vec3 xrPos = input->xr_right_eye_pos;
            glm::quat quat;
            quat.x = xrQuat[0];
            quat.y = xrQuat[1];
            quat.z = xrQuat[2];
            quat.w = xrQuat[3];

            glm::mat4 rotation = glm::toMat4(quat);
            glm::mat4 translation = glm::translate(glm::mat4(1.0f), xrPos);

            cameraR->SetTransform(
                entity->GetParent()->GetGlobalTransform() * translation * rotation);
        }

        technique->PushRendererData(cameraL);
        technique->PushRendererData(cameraR);
    }
    else if (state == Scene::State::Running)
    {
        std::shared_ptr<VulkanCamera> camera =
            vrDisplay->GetLeftCamera();

        ASSERT(camera != nullptr);

        camera->SetTransform(entity->GetGlobalTransform());
        technique->PushRendererData(camera);
    }
}

void VrDisplayComponent::Serialize(Json::Value& json)
{
    json["nothing"] = "nothing";
}

VrDisplayComponent::~VrDisplayComponent()
{
    vrDisplay = nullptr; // Free smart pointer
}

} // namespace renderer