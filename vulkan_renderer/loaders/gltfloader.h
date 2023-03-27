#pragma once

#include <json/json.h>

#include <string>
#include <vector>
#include <fstream>
#include <memory>

#include "vulkan_texture.h"
#include "vulkan_material.h"
#include "vulkan_node.h"
#include "vulkan_mesh.h"

namespace renderer
{

class GltfModel
{
    Json::Value gltf;

public:
    GltfModel LoadModel(std::string path);
    std::unique_ptr<VulkanNode> node;

private:
    void LoadBuffers();
    void LoadTextures();
    void LoadMaterials();
    void LoadMeshes(Node& parentNode, int meshIndex);
    void ProcessNode(Node& parentNode, int nodeIndex);

private:
   std::vector<std::vector<unsigned char>> bufferList;
   std::vector<std::shared_ptr<VulkanTexture>> textureList;
   std::vector<std::shared_ptr<VulkanMaterial>> materialList;
   std::vector<std::shared_ptr<VulkanMesh>> meshList;
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

} // namespace renderer