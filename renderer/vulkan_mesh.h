#pragma once

#include "mesh.h"

#include "vulkan_material.h"
#include "vk_primitives/vulkan_vertexbuffer.h"
#include "vk_primitives/vulkan_uniform.h"
#include "vk_primitives/vulkan_buffer.h"
#include "vk_primitives/vulkan_device.h"

#include "serialization.h"

#include <memory>
#include <tracy/Tracy.hpp>


namespace renderer
{

class VulkanMesh: public Mesh
{

public:
    static std::shared_ptr<Mesh> BuildMesh(BuildMeshInfo& info);

    void AddMaterial(std::shared_ptr<Material> material) override;

    void RemoveMaterial() override;

    void Serialize(Json::Value& json) override;

    std::string GetResourcePath() override;

    ~VulkanMesh() override;

    VulkanVertexbuffer& GetVertexbuffer();
    std::shared_ptr<VulkanMaterial> GetVulkanMaterial();

private:
    VulkanVertexbuffer vertexbuffer{};

    std::shared_ptr<Material> material;
    
    std::string resourcePath;
};

template<typename T>
class VulkanInstanceMesh: public Mesh
{

public:
    static std::shared_ptr<VulkanInstanceMesh> BuildMesh(
        BuildMeshInfo& info, VulkanDevice* vulkanDevice)
    {
        ZoneScopedN("VulkanInstanceMesh::BuildMesh");

        std::shared_ptr<VulkanInstanceMesh> mesh =
            std::make_shared<VulkanInstanceMesh>();
        
        mesh->vulkanDevice = vulkanDevice;

        mesh->vertexbuffer.Initialize(vulkanDevice,
            sizeof(unsigned int) * info.indices.size(),
            sizeof(Vertex) * info.vertices.size());

        void* indexData = mesh->vertexbuffer.MapIndex();
        void* vertexData = mesh->vertexbuffer.MapVertex();

        memcpy(indexData, info.indices.data(),
            sizeof(uint32_t) * info.indices.size());
        memcpy(vertexData, info.vertices.data(),
            sizeof(Vertex) * info.vertices.size());
        
        mesh->material = VulkanMaterial::GetDefaultMaterial();

        mesh->resourcePath = info.resourcePath;

        mesh->instanceBuffer = new VulkanBuffer<T>();
        mesh->instanceBuffer->Initialize(
            vulkanDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 8192);

        return mesh;
    }

    void AddMaterial(std::shared_ptr<Material> material) override
    {
        ZoneScopedN("VulkanInstanceMesh::AddMaterial");

        this->material = material;
    }

    void RemoveMaterial() override
    {
        ZoneScopedN("VulkanInstanceMesh::RemoveMaterial");

        this->material = VulkanMaterial::GetDefaultMaterial();
    }

    void Serialize(Json::Value& json) override
    {
        json[JSON_TYPE] = (int)JsonType::Mesh;

        // FIXME: should be different from regular mesh
        json["resourcePath"] = resourcePath;
        json["material"] = material?
            material->GetProperties()->resourcePath: "none";
    }

    std::string GetResourcePath() override
    {
        return resourcePath;
    }

    VulkanVertexbuffer& GetVertexbuffer()
    {
        return this->vertexbuffer;
    }

    std::shared_ptr<VulkanMaterial> GetVulkanMaterial()
    {
        return std::static_pointer_cast<VulkanMaterial>(this->material);
    }

    uint32_t GetInstanceCount()
    {
        return instanceBuffer->Size();
    }

    VulkanBuffer<T>* GetInstanceBuffer()
    {
        return instanceBuffer;
    }

    ~VulkanInstanceMesh() override
    {
        ZoneScopedN("VulkanInstanceMesh::~VulkanInstanceMesh");

        vkDeviceWaitIdle(vulkanDevice->vkDevice);

        vertexbuffer.Destroy();
        instanceBuffer->Destroy();
        delete instanceBuffer;
    }

private:
    VulkanDevice* vulkanDevice = nullptr;
    
    VulkanVertexbuffer vertexbuffer{};
    std::shared_ptr<Material> material;

    VulkanBuffer<T>* instanceBuffer;
    std::string resourcePath;
};
} // namespace renderer