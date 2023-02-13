#include "vulkan_mesh.h"

#include "vulkan_renderer.h"
#include "vk_primitives/vulkan_device.h"
#include "vk_primitives/vulkan_pipeline_layout.h"

#include <memory>


namespace renderer
{

std::shared_ptr<Mesh> VulkanMesh::BuildMesh(BuildMeshInfo& info)
{
    std::shared_ptr<VulkanMesh> mesh = std::make_shared<VulkanMesh>();
    VulkanRenderer& vkr = VulkanRenderer::GetInstance();
    VulkanDevice* vulkanDevice = &vkr.vulkanDevice;

    mesh->vertexbuffer.Initialize(vulkanDevice,
        sizeof(unsigned int) * info.indices.size(),
        sizeof(Vertex) * info.vertices.size());

    void* indexData = mesh->vertexbuffer.MapIndex();
    void* vertexData = mesh->vertexbuffer.MapVertex();

    int indexCount = info.indices.size();
    memcpy(indexData, info.indices.data(),
        sizeof(VertexIndex) * info.indices.size());
    memcpy(vertexData, info.vertices.data(),
        sizeof(Vertex) * info.vertices.size());

    mesh->uniform.Initialize(vulkanDevice, sizeof(glm::mat4));
    mesh->map = static_cast<glm::mat4*>(mesh->uniform.Map());

    VulkanPipelineLayout& layout = vkr.GetPipelineLayout("renderer");
    layout.AllocateDescriptorSet("mesh", vkr.FRAME_IN_FLIGHT, &mesh->descSet);

    VkWriteDescriptorSet descriptorWrite;
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = mesh->descSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = mesh->uniform.GetDescriptor();

    vkUpdateDescriptorSets(
        vulkanDevice->vkDevice, 1, &descriptorWrite, 0, nullptr);

    //TODO: material import
}

void VulkanMesh::AddMaterial(std::shared_ptr<Material> material)
{
    
}

void VulkanMesh::RemoveMaterial()
{
    
}

} // namespace renderer
