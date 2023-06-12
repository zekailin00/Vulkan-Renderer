#include "wireframe_component.h"

#include "entity.h"
#include "serialization.h" 


namespace renderer
{

Component* WireframeInitializer::operator()(Entity* entity)
{
    WireframeComponent* component = new WireframeComponent();
    component->entity = entity;
    component->type = Component::Type::Wireframe;
    component->technique = technique;

    component->wireframe = nullptr;

    return component;
}

Component* WireframeDeserializer::operator()(Entity* entity, Json::Value& json)
{
    WireframeComponent* component = new WireframeComponent();
    component->entity = entity;
    component->type = Component::Type::Wireframe;
    component->technique = technique;

    VulkanWireframe::WireframeType wireframeType =
        (VulkanWireframe::WireframeType)json["type"].asInt();

    glm::vec3 color;
    DeserializeVec3(color, json["color"]);
    float width = json["width"].asFloat();

    switch (wireframeType)
    {
    case VulkanWireframe::Line1:
        {
            glm::vec3 direction;
            float length;

            DeserializeVec3(direction, json["direction"]);
            length = json["length"].asFloat();

            component->wireframe = VulkanWireframeGenerator::GetLine(
                direction, length, color, width
            );
        }
        break;
    case VulkanWireframe::Line2:
        {
            glm::vec3 beginPoint;
            glm::vec3 endPoint;

            DeserializeVec3(beginPoint, json["beginPoint"]);
            DeserializeVec3(endPoint, json["endPoint"]);

            component->wireframe = VulkanWireframeGenerator::GetLine(
                beginPoint, endPoint, color, width
            );
        }
        break;
    case VulkanWireframe::Sphere:
        {
            glm::vec3 position;
            float radius;

            DeserializeVec3(position, json["position"]);
            radius = json["radius"].asFloat();

            component->wireframe = VulkanWireframeGenerator::GetSphere(
                position, radius, color, width
            );
        }
        break;
    case VulkanWireframe::Circle:
        {
            glm::vec3 position;
            glm::vec3 normal;
            float radius;

            DeserializeVec3(position, json["position"]);
            DeserializeVec3(normal, json["normal"]);
            radius = json["radius"].asFloat();

            component->wireframe = VulkanWireframeGenerator::GetCircle(
                position, normal, radius, color, width
            );
        }
        break;
    case VulkanWireframe::AABB:
        {
            glm::vec3 minCoordinates;
            glm::vec3 maxCoordinates;

            DeserializeVec3(minCoordinates, json["minCoordinates"]);
            DeserializeVec3(maxCoordinates, json["maxCoordinates"]);

            component->wireframe = VulkanWireframeGenerator::GetAABB(
                minCoordinates, maxCoordinates, color, width
            );
        }
        break;
    case VulkanWireframe::OBB:
        {
            glm::mat4 transform;

            DeserializeMat4(transform, json["transform"]);

            component->wireframe = VulkanWireframeGenerator::GetOBB(
                transform, color, width
            );
        }
        break;
    default:
        throw;
    }

    return component;
}

void WireframeComponent::Update(Timestep ts)
{
    if(!wireframe)
        return;
    
    technique->PushRendererData(wireframe->wireList);
}

void WireframeComponent::Serialize(Json::Value& json)
{
    json["type"]  = wireframe->type;
    SerializeVec3(wireframe->color, json["color"]);
    json["width"] = wireframe->width;

    switch (wireframe->type)
    {
    case VulkanWireframe::Line1:
        {
            std::shared_ptr<WireframeLine1> wireframeLine1 =
                std::dynamic_pointer_cast<WireframeLine1>(wireframe);
            SerializeVec3(wireframeLine1->direction, json["direction"]);
            json["length"] = wireframeLine1->length;
        }
        break;
    case VulkanWireframe::Line2:
        {
            std::shared_ptr<WireframeLine2> wireframeLine2 =
                std::dynamic_pointer_cast<WireframeLine2>(wireframe);
            SerializeVec3(wireframeLine2->beginPoint, json["beginpoint"]);
            SerializeVec3(wireframeLine2->endPoint, json["endPoint"]);
        }
        break;
    case VulkanWireframe::Sphere:
        {
            std::shared_ptr<WireframeSphere> wireframeSphere =
                std::dynamic_pointer_cast<WireframeSphere>(wireframe);
            SerializeVec3(wireframeSphere->position, json["position"]);
            json["radius"] = wireframeSphere->radius;
        }
        break;
    case VulkanWireframe::Circle:
        {
            std::shared_ptr<WireframeCircle> wireframeCircle =
                std::dynamic_pointer_cast<WireframeCircle>(wireframe);
            SerializeVec3(wireframeCircle->position, json["position"]);
            SerializeVec3(wireframeCircle->normal, json["normal"]);
            json["radius"] = wireframeCircle->radius;
        }
        break;
    case VulkanWireframe::AABB:
        {
            std::shared_ptr<WireframeAABB> wireframeAABB =
                std::dynamic_pointer_cast<WireframeAABB>(wireframe);
            SerializeVec3(wireframeAABB->minCoordinates, json["minCoordinates"]);
            SerializeVec3(wireframeAABB->maxCoordinates, json["maxCoordinates"]);
        }
        break;
    case VulkanWireframe::OBB:
        {
            std::shared_ptr<WireframeOBB> wireframeOBB =
                std::dynamic_pointer_cast<WireframeOBB>(wireframe);
            SerializeMat4(wireframeOBB->transform, json["transform"]);
        }
        break;
    default:
        throw;
    }
}

WireframeComponent::~WireframeComponent()
{
    wireframe = nullptr; // free the smart pointer
}

} // namespace renderer