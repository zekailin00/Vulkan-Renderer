#include "vulkan_material.h"

#include "vulkan_renderer.h"
#include "vk_primitives/vulkan_pipeline_layout.h"

#include <vector>


namespace renderer
{

std::shared_ptr<VulkanMaterial> VulkanMaterial::defaultMaterial;

std::shared_ptr<Material> VulkanMaterial::BuildMaterial(MaterialProperties* prop)
{
    VulkanRenderer& vkr = VulkanRenderer::GetInstance();

    std::shared_ptr<VulkanMaterial> material = std::make_shared<VulkanMaterial>();

    material->vulkanDevice = &vkr.vulkanDevice;
    material->uniform.Initialize(material->vulkanDevice, sizeof(MaterialUniform));
    material->map = static_cast<MaterialUniform*>(material->uniform.Map());

    material->map->useAlbedoTex = (prop->albedoTexture != nullptr);
    material->map->useMetallicTex = (prop->metallicTexture != nullptr);
    material->map->useSmoothnessTex = (prop->smoothnessTexture != nullptr);
    material->map->useNormalTex = (prop->normalTexture != nullptr);
    material->map->albedo = glm::vec4(prop->albedo, 0);
    material->map->metallic = prop->metallic;
    material->map->smoothness = prop->smoothness;

    material->properties = *prop;
    
    VulkanPipelineLayout& layout = vkr.GetPipelineLayout("render");

    layout.AllocateDescriptorSet(
        "material",
        vkr.FRAME_IN_FLIGHT,
        &material->descriptorSet);

    std::vector<VkWriteDescriptorSet> descWrites;

    {
        VkWriteDescriptorSet descWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descWrite.dstSet = material->descriptorSet;
        descWrite.dstBinding = 0;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descWrite.descriptorCount = 1;
        descWrite.pBufferInfo = material->uniform.GetDescriptor();
        descWrites.push_back(descWrite);
    }

    {
        VkWriteDescriptorSet descWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descWrite.dstSet = material->descriptorSet;
        descWrite.dstBinding = 1;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrite.descriptorCount = 1;

        if (material->map->useAlbedoTex)
        {
            std::shared_ptr<VulkanTexture> vkt = std::dynamic_pointer_cast<VulkanTexture>(
                material->properties.albedoTexture);
            descWrite.pImageInfo = vkt->GetDescriptor();
        } else {
            descWrite.pImageInfo = VulkanTexture::GetDefaultTexture()->GetDescriptor();
        }
        descWrites.push_back(descWrite);
    }

    {
        VkWriteDescriptorSet descWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descWrite.dstSet = material->descriptorSet;
        descWrite.dstBinding = 2;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrite.descriptorCount = 1;

        if (material->map->useMetallicTex)
        {
            std::shared_ptr<VulkanTexture> vkt = std::dynamic_pointer_cast<VulkanTexture>(
                material->properties.metallicTexture);
            descWrite.pImageInfo = vkt->GetDescriptor();
        } else {
            descWrite.pImageInfo = VulkanTexture::GetDefaultTexture()->GetDescriptor();
        }
        descWrites.push_back(descWrite);
    }

    {
        VkWriteDescriptorSet descWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descWrite.dstSet = material->descriptorSet;
        descWrite.dstBinding = 3;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrite.descriptorCount = 1;

        if (material->map->useSmoothnessTex)
        {
            std::shared_ptr<VulkanTexture> vkt = std::dynamic_pointer_cast<VulkanTexture>(
                material->properties.smoothnessTexture);
            descWrite.pImageInfo = vkt->GetDescriptor();
        } else {
            descWrite.pImageInfo = VulkanTexture::GetDefaultTexture()->GetDescriptor();
        }
        descWrites.push_back(descWrite);
    }

    {
        VkWriteDescriptorSet descWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descWrite.dstSet = material->descriptorSet;
        descWrite.dstBinding = 4;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrite.descriptorCount = 1;

        if (material->map->useNormalTex)
        {
            std::shared_ptr<VulkanTexture> vkt = std::dynamic_pointer_cast<VulkanTexture>(
                material->properties.normalTexture);
            descWrite.pImageInfo = vkt->GetDescriptor();
        } else {
            descWrite.pImageInfo = VulkanTexture::GetDefaultTexture()->GetDescriptor();
        }
        descWrites.push_back(descWrite);
    }

    vkUpdateDescriptorSets(
        material->vulkanDevice->vkDevice,
        descWrites.size(), descWrites.data(),
        0, nullptr);

    return material;
}

std::shared_ptr<Material> VulkanMaterial::GetDefaultMaterial()
{
    if (defaultMaterial == nullptr)
    {
        MaterialProperties defaultProperites{};
        defaultMaterial = std::dynamic_pointer_cast<VulkanMaterial>
            (BuildMaterial(&defaultProperites));
    }

    return defaultMaterial;
}

MaterialProperties* VulkanMaterial::GetProperties()
{
    return &properties;
}

void VulkanMaterial::ResetProperties()
{
    ResetAlbedoTexture();
    ResetMetallicTexture();
    ResetSmoothnessTexture();
    ResetNormalTexture();
}

void VulkanMaterial::AddAlbedoTexture(std::shared_ptr<Texture> texture)
{
    if(this->properties.albedoTexture != nullptr)
    {
        vkDeviceWaitIdle(vulkanDevice->vkDevice);
    }

    map->useAlbedoTex = 1.0f;
    properties.albedoTexture = texture;

    std::shared_ptr<VulkanTexture> vkt =
        std::dynamic_pointer_cast<VulkanTexture>(texture);

    VkWriteDescriptorSet descWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descWrite.dstSet = this->descriptorSet;
    descWrite.dstBinding = 1;
    descWrite.dstArrayElement = 0;
    descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descWrite.descriptorCount = 1;
    descWrite.pImageInfo = vkt->GetDescriptor();

    vkUpdateDescriptorSets(
        this->vulkanDevice->vkDevice, 1, &descWrite, 0, nullptr);
}

void VulkanMaterial::AddMetallicTexture(std::shared_ptr<Texture> texture)
{
    if(this->properties.metallicTexture != nullptr)
    {
        vkDeviceWaitIdle(vulkanDevice->vkDevice);
    }

    map->useMetallicTex = 1.0f;
    properties.metallicTexture = texture;

    std::shared_ptr<VulkanTexture> vkt =
        std::dynamic_pointer_cast<VulkanTexture>(texture);

    VkWriteDescriptorSet descWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descWrite.dstSet = this->descriptorSet;
    descWrite.dstBinding = 2;
    descWrite.dstArrayElement = 0;
    descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descWrite.descriptorCount = 1;
    descWrite.pImageInfo = vkt->GetDescriptor();

    vkUpdateDescriptorSets(
        this->vulkanDevice->vkDevice, 1, &descWrite, 0, nullptr);
}

void VulkanMaterial::AddSmoothnessTexture(std::shared_ptr<Texture> texture)
{
    if(this->properties.smoothnessTexture != nullptr)
    {
        vkDeviceWaitIdle(vulkanDevice->vkDevice);
    }

    map->useSmoothnessTex = 1.0f;
    properties.smoothnessTexture = texture;

    std::shared_ptr<VulkanTexture> vkt =
        std::dynamic_pointer_cast<VulkanTexture>(texture);

    VkWriteDescriptorSet descWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descWrite.dstSet = this->descriptorSet;
    descWrite.dstBinding = 3;
    descWrite.dstArrayElement = 0;
    descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descWrite.descriptorCount = 1;
    descWrite.pImageInfo = vkt->GetDescriptor();

    vkUpdateDescriptorSets(
        this->vulkanDevice->vkDevice, 1, &descWrite, 0, nullptr);
}

void VulkanMaterial::AddNormalTexture(std::shared_ptr<Texture> texture)
{
    if(this->properties.normalTexture != nullptr)
    {
        vkDeviceWaitIdle(vulkanDevice->vkDevice);
    }

    map->useNormalTex = 1.0f;
    properties.normalTexture = texture;

    std::shared_ptr<VulkanTexture> vkt =
        std::dynamic_pointer_cast<VulkanTexture>(texture);

    VkWriteDescriptorSet descWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descWrite.dstSet = this->descriptorSet;
    descWrite.dstBinding = 4;
    descWrite.dstArrayElement = 0;
    descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descWrite.descriptorCount = 1;
    descWrite.pImageInfo = vkt->GetDescriptor();

    vkUpdateDescriptorSets(
        this->vulkanDevice->vkDevice, 1, &descWrite, 0, nullptr);
}

void VulkanMaterial::ResetAlbedoTexture()
{
    if (this->properties.albedoTexture != nullptr)
    {
        vkDeviceWaitIdle(vulkanDevice->vkDevice);

        VkWriteDescriptorSet descWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descWrite.dstSet = this->descriptorSet;
        descWrite.dstBinding = 1;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrite.descriptorCount = 1;
        descWrite.pImageInfo = 
            VulkanTexture::GetDefaultTexture()->GetDescriptor();

        vkUpdateDescriptorSets(
            this->vulkanDevice->vkDevice,
            1, &descWrite, 0, nullptr);

        this->properties.albedoTexture = nullptr;
    }

    this->properties.albedo = {255, 255, 255};
    this->map->albedo = glm::vec4(this->properties.albedo, 0);
    this->map->useAlbedoTex = 0.0f;
}

void VulkanMaterial::ResetMetallicTexture()
{
    if (this->properties.metallicTexture != nullptr)
    {
        vkDeviceWaitIdle(vulkanDevice->vkDevice);

        VkWriteDescriptorSet descWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descWrite.dstSet = this->descriptorSet;
        descWrite.dstBinding = 2;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrite.descriptorCount = 1;
        descWrite.pImageInfo = 
            VulkanTexture::GetDefaultTexture()->GetDescriptor();

        vkUpdateDescriptorSets(
            this->vulkanDevice->vkDevice,
            1, &descWrite, 0, nullptr);

        this->properties.metallicTexture = nullptr;
    }

    this->properties.metallic = 0.0f;
    this->map->metallic = this->properties.metallic;
    this->map->useMetallicTex = 0.0f; 
}

void VulkanMaterial::ResetSmoothnessTexture()
{
    if (this->properties.smoothnessTexture != nullptr)
    {
        vkDeviceWaitIdle(vulkanDevice->vkDevice);

        VkWriteDescriptorSet descWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descWrite.dstSet = this->descriptorSet;
        descWrite.dstBinding = 1;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrite.descriptorCount = 1;
        descWrite.pImageInfo = 
            VulkanTexture::GetDefaultTexture()->GetDescriptor();

        vkUpdateDescriptorSets(
            this->vulkanDevice->vkDevice,
            1, &descWrite, 0, nullptr);

        this->properties.smoothnessTexture = nullptr;
    }

    this->properties.smoothness = 0.5f;
    this->map->smoothness = this->properties.smoothness;
    this->map->useSmoothnessTex = 0.0f;
}

void VulkanMaterial::ResetNormalTexture()
{
    if (this->properties.normalTexture != nullptr)
    {
        vkDeviceWaitIdle(vulkanDevice->vkDevice);

        VkWriteDescriptorSet descWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descWrite.dstSet = this->descriptorSet;
        descWrite.dstBinding = 1;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrite.descriptorCount = 1;
        descWrite.pImageInfo = 
            VulkanTexture::GetDefaultTexture()->GetDescriptor();

        vkUpdateDescriptorSets(
            this->vulkanDevice->vkDevice,
            1, &descWrite, 0, nullptr);

        this->properties.normalTexture = nullptr;
    }

    this->map->useAlbedoTex = 0.0f;
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
