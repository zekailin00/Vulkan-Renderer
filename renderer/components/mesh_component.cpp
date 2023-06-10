#include "mesh_component.h"

#include "vulkan_renderer.h"
#include "validation.h"
#include "serialization.h"

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
    ASSERT(json[JSON_TYPE].asInt() == (int)JsonType::Mesh);

    std::string resourcePath = json["resourcePath"].asString();
    std::string materialPath = json["material"].asString();

    MeshComponent* component = new MeshComponent();
    component->entity = entity;
    component->type = Component::Type::Mesh;
    component->technique = technique;
    component->mesh = std::dynamic_pointer_cast<VulkanMesh>(
        renderer->GetAssetManager()->GetMesh(resourcePath));

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
    mesh->Serialize(json);
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
