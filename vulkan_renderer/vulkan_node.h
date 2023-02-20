#pragma once

#include "node.h"

#include "vk_primitives/vulkan_uniform.h"
#include "vk_primitives/vulkan_device.h"

#include "vulkan_renderer.h"

namespace renderer
{

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


    glm::mat4 GetTransform() override;
    void SetTransform(glm::mat4 transform) override;

    /**
     * There is no builder for VulkanNode.
     * Make a unique pointer directly
     * and constructor does all the work.
    */
    VulkanNode();
    ~VulkanNode() override;

private:
    VulkanUniform uniform{};
    VkDescriptorSet descSet = VK_NULL_HANDLE;
    VulkanDevice* vulkanDevice = nullptr;
    glm::mat4* transform = nullptr;
};

} // namespace renderer
