#pragma once

#include <json/json.h>

#include <string>
#include <vector>
#include <fstream>
#include <memory>

#include "vulkan_texture.h"
#include "vulkan_material.h"
#include "vulkan_mesh.h"
#include "stb/stb_image.h"

#include "scene.h"

class GltfModel
{
public:
    struct PixelData
    {
        stbi_uc* pixels;
        int width, height;
        PixelData(stbi_uc* ptr, int w, int h)
            :pixels(ptr), width(w), height(h) {}
        ~PixelData()
        {
            stbi_image_free(pixels);
        }
    };

public:
    /**
     * @brief Import a GLTF model from the path.
     * 
     * @param path Absolute path to a model in valid gltf format.
     * @return GltfModel 
     */
    static std::shared_ptr<GltfModel> Import(std::string path, Scene* scene);

    const Json::Value&                                            GetModel();
    const std::vector<std::vector<unsigned char>>&                GetBufferList();
    // WARNING: could contain nullptr FIXME:
    const std::vector<std::shared_ptr<renderer::BuildMeshInfo>>&  GetMeshInfoList();
    const std::vector<std::shared_ptr<renderer::VulkanMesh>>&     GetMeshList();
    const std::vector<std::shared_ptr<renderer::VulkanMaterial>>& GetMaterialList();
    const std::vector<std::shared_ptr<renderer::VulkanTexture>>&  GetTextureList();
    const std::vector<std::shared_ptr<renderer::VulkanTexture>>&  GetRoughTexList();
    const std::vector<std::shared_ptr<renderer::VulkanTexture>>&  GetMetalTexList();
    const std::vector<std::shared_ptr<PixelData>>&                GetPixelDataList();
    const std::vector<std::shared_ptr<PixelData>>&                GetRoughPixDataList();
    const std::vector<std::shared_ptr<PixelData>>&                GetMetalPixDataList();

    Entity* GetModelEntity() {return modelEntity;} //FIXME: same issue

private:
    void LoadBuffers();
    void LoadTextures();
    void LoadMaterials();
    void LoadMeshes(Entity* parentEntity, int meshIndex);
    void ProcessNode(Entity* parentEntity, int nodeIndex);

private:
    Json::Value gltf;       // GLTF format defined by its spec
    Json::Value model;      // Format defined by the engine
    Scene* scene;           //FIXME: needs prefab system before solving this.
    Entity* modelEntity;    // FIXME: same issue.

    std::vector<std::vector<unsigned char>>                 bufferList;
    std::vector<std::shared_ptr<renderer::BuildMeshInfo>>   meshInfoList;
    std::vector<std::shared_ptr<renderer::VulkanMesh>>      meshList;
    std::vector<std::shared_ptr<renderer::VulkanMaterial>>  materialList;
    std::vector<std::shared_ptr<renderer::VulkanTexture>>   textureList;
    std::vector<std::shared_ptr<renderer::VulkanTexture>>   roughTexList;
    std::vector<std::shared_ptr<renderer::VulkanTexture>>   metalTexList;
    std::vector<std::shared_ptr<PixelData>>                 pixelDataList;
    std::vector<std::shared_ptr<PixelData>>                 roughPixDataList;
    std::vector<std::shared_ptr<PixelData>>                 metalPixDataList;

    // Ensure names saved to the filesystem are unique
    // the second element is next available number
    std::map<std::string, int> namePool{};
};

enum SamplerType
{
    NEAREST                = 9728,
    LINEAR                 = 9729,
    NEAREST_MIPMAP_NEAREST = 9984,
    LINEAR_MIPMAP_NEAREST  = 9985,
    NEAREST_MIPMAP_LINEAR  = 9986,
    LINEAR_MIPMAP_LINEAR   = 9987
};

enum DataType
{
    SIGNED_BYTE    = 5120, // 8 bits
    UNSIGNED_BYTE  = 5121, // 8 bits
    SIGNED_SHORT   = 5122, // 16 bits
    UNSIGNED_SHORT = 5123, // 16 bits
    SIGNED_INT     = 5125, // 32 bits
    FLOAT          = 5126  // 32 bits
};