#include "vulkan_mesh.h"

#include "vulkan_renderer.h"
#include "vulkan_material.h"

#include "vk_primitives/vulkan_device.h"
#include "vk_primitives/vulkan_pipeline_layout.h"

#include "serialization.h"

#include <memory>
#include <tracy/Tracy.hpp>


namespace renderer
{

std::shared_ptr<Mesh> VulkanMesh::BuildMesh(BuildMeshInfo& info)
{
    ZoneScopedN("VulkanMesh::BuildMesh");

    std::shared_ptr<VulkanMesh> mesh = std::make_shared<VulkanMesh>();
    VulkanRenderer& vkr = VulkanRenderer::GetInstance();
    VulkanDevice* vulkanDevice = &vkr.vulkanDevice;

    mesh->vertexbuffer.Initialize(vulkanDevice,
        sizeof(unsigned int) * info.indices.size(),
        sizeof(Vertex) * info.vertices.size());

    void* indexData = mesh->vertexbuffer.MapIndex();
    void* vertexData = mesh->vertexbuffer.MapVertex();

    memcpy(indexData, info.indices.data(),
        sizeof(VertexIndex) * info.indices.size());
    memcpy(vertexData, info.vertices.data(),
        sizeof(Vertex) * info.vertices.size());

    // mesh->uniform.Initialize(vulkanDevice, sizeof(glm::mat4));
    // mesh->map = static_cast<glm::mat4*>(mesh->uniform.Map());

    // VulkanPipelineLayout& layout = vkr.GetPipelineLayout("render");
    // layout.AllocateDescriptorSet("mesh", vkr.FRAME_IN_FLIGHT, &mesh->descSet);

    // VkWriteDescriptorSet descriptorWrite{};
    // descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // descriptorWrite.dstSet = mesh->descSet;
    // descriptorWrite.dstBinding = 0;
    // descriptorWrite.dstArrayElement = 0;
    // descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // descriptorWrite.descriptorCount = 1;
    // descriptorWrite.pBufferInfo = mesh->uniform.GetDescriptor();

    // vkUpdateDescriptorSets(
    //     vulkanDevice->vkDevice, 1, &descriptorWrite, 0, nullptr);

    mesh->material = VulkanMaterial::GetDefaultMaterial();

    mesh->resourcePath = info.resourcePath;
    return mesh;
}

void VulkanMesh::AddMaterial(std::shared_ptr<Material> material)
{
    ZoneScopedN("VulkanMesh::AddMaterial");

    this->material = material;
}

void VulkanMesh::RemoveMaterial()
{
    ZoneScopedN("VulkanMesh::RemoveMaterial");

    this->material = VulkanMaterial::GetDefaultMaterial();
}

void VulkanMesh::Serialize(Json::Value& json)
{
    json[JSON_TYPE] = (int)JsonType::Mesh;

    json["resourcePath"] = resourcePath;
    json["material"] = material?
        material->GetProperties()->resourcePath: "none";
}

std::string VulkanMesh::GetResourcePath()
{
    return resourcePath;
}

VulkanVertexbuffer& VulkanMesh::GetVertexbuffer()
{
    ZoneScopedN("VulkanMesh::GetVertexbuffer");

    return this->vertexbuffer;
}

std::shared_ptr<VulkanMaterial> VulkanMesh::GetVulkanMaterial()
{
    ZoneScopedN("VulkanMesh::GetVulkanMaterial");

    return std::static_pointer_cast<VulkanMaterial>(this->material);;
}

VulkanMesh::~VulkanMesh()
{
    ZoneScopedN("VulkanMesh::~VulkanMesh");

    VulkanRenderer& vkr = VulkanRenderer::GetInstance();
    VulkanDevice* vulkanDevice = &vkr.vulkanDevice;
    vkDeviceWaitIdle(vulkanDevice->vkDevice);

    vertexbuffer.Destroy();
}

 std::shared_ptr<Mesh> VulkanInstanceMesh::BuildMesh(BuildMeshInfo& info)
 {
    VulkanRenderer& vkr = VulkanRenderer::GetInstance();
    VulkanDevice* vulkanDevice = &vkr.vulkanDevice;

    std::shared_ptr<VulkanInstanceMesh> mesh =
        std::make_shared<VulkanInstanceMesh>();
    
    mesh->transformBuffer = new VulkanBuffer<glm::mat4>(vulkanDevice, true);
    mesh->transformBuffer->Initialize(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0);
 }

} // namespace renderer
