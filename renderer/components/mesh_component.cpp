#include "mesh_component.h"

#include "vulkan_renderer.h"

namespace renderer
{

Component* MeshInitializer::operator()(Entity* entity)
{
    MeshComponent* component = new MeshComponent();
    component->entity = entity;
    component->type = Component::Type::Mesh;
    component->technique = technique;
    component->mesh = nullptr;

    component->vulkanDevice = &renderer->vulkanDevice;
    component->uniform.Initialize(&renderer->vulkanDevice, sizeof(glm::mat4));
    component->transform = static_cast<glm::mat4*>(component->uniform.Map());

    VulkanPipelineLayout& layout = renderer->GetPipelineLayout("render");
    layout.AllocateDescriptorSet("mesh", renderer->FRAME_IN_FLIGHT, &component->descSet);
    
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = component->descSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = component->uniform.GetDescriptor();

    vkUpdateDescriptorSets(
        renderer->vulkanDevice.vkDevice, 1, &descriptorWrite, 0, nullptr);

    return component;
}

Component* MeshDeserializer::operator()(Entity* entity, Json::Value& json)
{
    throw; //TODO: a lot of work...
    return nullptr;
}

void MeshComponent::Update(Timestep ts)
{
    if (!mesh)
        return;

    *transform = entity->GetGlobalTransform();
    RenderTechnique::MeshPacket packet{mesh, descSet};

    technique->PushRendererData(packet);
}

void MeshComponent::Serialize(Json::Value& json)
{
    throw; //TODO: a lot of work to do
}

MeshComponent::~MeshComponent()
{
    vkDeviceWaitIdle(vulkanDevice->vkDevice);
    uniform.Destroy();
    vulkanDevice = nullptr;
    transform = nullptr;

    mesh = nullptr; // free the smart pointer.
}

} // namespace renderer
