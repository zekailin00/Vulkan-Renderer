#include "vulkan_material.h"

#include "vulkan_renderer.h"
#include "vk_primitives/vulkan_pipeline_layout.h"

#include <vector>


namespace renderer
{

std::shared_ptr<Material> VulkanMaterial::BuildMaterial(MaterialProperties* prop)
{
    VulkanRenderer& vkr = VulkanRenderer::GetInstance();

    std::shared_ptr<VulkanMaterial> material = std::make_shared<VulkanMaterial>();

    material->vulkanDevice = &vkr.vulkanDevice;
    material->uniform.Initialize(material->vulkanDevice, sizeof(MaterialUniform));
    material->map = static_cast<MaterialUniform*>(material->uniform.Map());

    material->map->useAlbedoTex = (prop->albedoTexture != nullptr);
    material->map->useMetallicTex = (prop->metalicTexture != nullptr);
    material->map->useSmoothnessTex = (prop->smoothnessTexture != nullptr);
    material->map->useNormalTex = (prop->normalTexture != nullptr);

    material->map->albedo = prop->albedo;
    material->map->metallic = prop->metallic;
    material->map->smoothness = prop->smoothness;

    material->albedoTex = prop->albedoTexture;
    material->metallicTex = prop->metalicTexture;
    material->smoothnessTex = prop->smoothnessTexture;
    material->normalTex = prop->normalTexture;
    
    VulkanPipelineLayout& layout = vkr.GetPipelineLayout("render");

    layout.AllocateDescriptorSet(
        "material",
        vkr.FRAME_IN_FLIGHT,
        &material->descriptorSet);

    std::vector<VkWriteDescriptorSet> descWrites;
    VkDescriptorBufferInfo descInfo = material->uniform.GetDescriptor();
    VkWriteDescriptorSet descWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descWrite.dstSet = material->descriptorSet;
    descWrite.dstBinding = 0;
    descWrite.dstArrayElement = 0;
    descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descWrite.descriptorCount = 1;
    descWrite.pBufferInfo = &descInfo;
    descWrites.push_back(descWrite);

    vkUpdateDescriptorSets(
        material->vulkanDevice->vkDevice,
        descWrites.size(), descWrites.data(),
        0, nullptr);

    return material;
}

void VulkanMaterial::AddAlbedoTexture(std::shared_ptr<Texture> texture)
{

}

void VulkanMaterial::AddMetallicTexture(std::shared_ptr<Texture> texture)
{

}

void VulkanMaterial::AddSmoothnessTexture(std::shared_ptr<Texture> texture)
{

}

void VulkanMaterial::AddNormalTexture(std::shared_ptr<Texture> texture)
{

}

void VulkanMaterial::Destory()
{
    vkDeviceWaitIdle(vulkanDevice->vkDevice);
    uniform.Destroy();
    this->map = nullptr;
}

VulkanMaterial::~VulkanMaterial()
{
    Destory();
}

} // namespace renderer
