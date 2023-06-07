#include "mesh_component.h"

namespace renderer
{

Component* MeshInitializer::operator()(Entity* entity)
{
    return nullptr;
}

Component* MeshDeserializer::operator()(Entity* entity, Json::Value& json)
{

}

void MeshComponent::Update(Timestep ts)
{
    
}

void MeshComponent::Serialize(Json::Value& json)
{

}

MeshComponent::~MeshComponent()
{

}

} // namespace renderer
