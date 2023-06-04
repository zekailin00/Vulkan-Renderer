#pragma once

#include "vulkan_node.h"
#include "vk_primitives/vulkan_uniform.h"
#include "vk_primitives/vulkan_vertexbuffer.h"

#include "vulkan_camera.h"
#include "vulkan_light.h"
#include "vulkan_mesh.h"
#include "vulkan_wireframe.h"
#include "vulkan_ui.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace renderer
{

class RenderTechnique
{
public:
    RenderTechnique() = default;
    ~RenderTechnique();
    void Destroy();

    RenderTechnique(const RenderTechnique&) = delete;
    RenderTechnique& operator=(const RenderTechnique&) = delete;

    void Initialize(VulkanDevice* vulkanDevice);
    void ProcessScene(VulkanNode* node);
    void ExecuteCommand(VkCommandBuffer commandBuffer);
    VkDescriptorSet GetDisplayDescSet() {return display;}
    VkDescriptorSet* GetXrDisplayDescSet() {return xrDisplay;}


private:
    void ScanNode(VulkanNode* node, const glm::mat4& transform);
    void DrawCamera(VkCommandBuffer vkCommandBuffer);

private:

    struct MeshPacket
    {
        std::shared_ptr<VulkanMesh> mesh;
        VkDescriptorSet descSet; // Mesh transform
        // FIXME: descset is not protected by shared_ptr
        // freee std::__ptr node can have memory access error. 
    };

    struct SceneData
    { // only the first element is nLight is used.
        glm::uvec4 nLight; // dirLight index has to be less that nLight
        DirLight dirLight[5];
    };

    /**
     * Using shared_ptr can resolve resource deallocation issue that
     * CPU can free resourses submitted to GPU command buffer for later rendering/
    */
    std::vector<MeshPacket> renderMesh{};
    std::vector<std::shared_ptr<VulkanCamera>> cameraList{};
    std::vector<WirePushConst> wireList{};
    std::vector<std::shared_ptr<VulkanUI>> uiList{};


    VulkanUniform sceneUniform{};
    VkDescriptorSet sceneDescSet;
    SceneData *sceneMap = nullptr; // 5 directional light elements

    VkDescriptorSet xrDisplay[2];
    VkDescriptorSet display; // The image that will be displayed in application
    VkDescriptorSet defaultDisplay; // Image that will be displayed when no camera is created.

    bool defaultSkybox = true;
    VkDescriptorSet skyboxTex;
    std::shared_ptr<VulkanMesh> skyboxMesh = nullptr;
    std::shared_ptr<VulkanTextureCube> textureCube = nullptr;
};

} // namespace renderer