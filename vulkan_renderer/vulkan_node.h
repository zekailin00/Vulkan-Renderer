#pragma once

#include "node.h"
#include "vulkan_wireframe.h"

#include "vk_primitives/vulkan_uniform.h"
#include "vk_primitives/vulkan_device.h"



namespace renderer
{

// Forward declaration
class RenderTechnique;

class VulkanNode: public Node
{

public:
    std::shared_ptr<Mesh> GetMesh() override;

    void SetMesh(std::shared_ptr<Mesh> mesh) override;

    std::shared_ptr<Camera> GetCamera() override;

    void SetCamera(std::shared_ptr<Camera> camera) override;

    std::shared_ptr<Light> GetLight() override;

    void SetLight(std::shared_ptr<Light> light) override;


    Node* AddChildNode(std::unique_ptr<Node> node) override;
    std::unique_ptr<Node> RemoveChildNode(Node* node) override;
    Node* GetChildNode(unsigned int index) override;


    const glm::mat4& GetTransform() override;
    void SetTransform(glm::mat4 transform) override;

    /**
     * There is no builder for VulkanNode.
     * Make a unique pointer directly
     * and constructor does all the work.
    */
    VulkanNode();
    ~VulkanNode() override;

    friend RenderTechnique;

    std::vector<WirePushConst> wireList;
    std::shared_ptr<UI> ui;

private:
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Light> light;
    std::shared_ptr<Camera> camera;

    VulkanUniform uniform{};
    VkDescriptorSet descSet = VK_NULL_HANDLE;
    VulkanDevice* vulkanDevice = nullptr;
    glm::mat4* transform = nullptr;

    glm::mat4 localTransform;
};

} // namespace renderer
