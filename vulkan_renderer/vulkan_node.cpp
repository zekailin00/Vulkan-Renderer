#include "vulkan_node.h"

#include "vulkan_camera.h"
#include "vulkan_light.h"

#include <memory>
#include <utility> // std::move

namespace renderer
{

std::shared_ptr<Mesh> VulkanNode::GetMesh()
{
    return mesh;
}

void VulkanNode::SetMesh(std::shared_ptr<Mesh> mesh)
{
    if (mesh != nullptr &&
        (this->mesh || this->camera || this->light))
        throw;
    this->mesh = mesh;
}

std::shared_ptr<Camera> VulkanNode::GetCamera()
{
    return camera;
}

void VulkanNode::SetCamera(std::shared_ptr<Camera> camera)
{
    if ((camera != nullptr) &&
        (this->mesh || this->camera || this->light))
        throw;
    this->camera = camera;
    this->SetTransform(*this->transform); // camera
}

std::shared_ptr<Light> VulkanNode::GetLight()
{
    return light;
}

void VulkanNode::SetLight(std::shared_ptr<Light> light)
{
    if (light != nullptr &&
        (this->mesh || this->camera || this->light))
        throw;
    this->light = light;
    this->SetTransform(*this->transform); // camera
}

Node* VulkanNode::AddChildNode(std::unique_ptr<Node> node)
{
    if (node)
    {
        Node* retval = &(*node);
        nodeLists.push_back(std::move(node));
        return retval;
    }

    return nullptr;
}

std::unique_ptr<Node> VulkanNode::RemoveChildNode(Node* node)
{
    std::unique_ptr<Node> removedNode;

    for (auto& e: nodeLists)
    {// FIXME: needs to be tested.
        if (&(*e) == node)
        {
            removedNode = std::move(e);
            nodeLists.remove(e);
            return removedNode;
        }
    }
    return nullptr;
}

Node* VulkanNode::GetChildNode(unsigned int index)
{
    for (auto& e: nodeLists)
    {
        if (index == 0)
            return &(*e);
        index--;
    }

    return nullptr;
}

glm::mat4 VulkanNode::GetTransform()
{
    return *transform;
}

void VulkanNode::SetTransform(glm::mat4 transform)
{
    *this->transform = transform;

    if (this->camera)
    {
        std::shared_ptr<VulkanCamera> vkCamera =
            std::dynamic_pointer_cast<VulkanCamera>(this->camera);
        vkCamera->SetTransform(*this->transform);
    }
    else if (this->light)
    {
        std::shared_ptr<VulkanLight> vkLight =
            std::dynamic_pointer_cast<VulkanLight>(this->light);
        vkLight->SetTransform(*this->transform);
    }
}

VulkanNode::VulkanNode()
{
    this->vulkanDevice = &VulkanRenderer::GetInstance().vulkanDevice;
    uniform.Initialize(this->vulkanDevice, sizeof(glm::mat4));
    transform = static_cast<glm::mat4*>(uniform.Map());
    
    VkWriteDescriptorSet descriptorWrite;
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = this->descSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = this->uniform.GetDescriptor();

    vkUpdateDescriptorSets(
        vulkanDevice->vkDevice, 1, &descriptorWrite, 0, nullptr);
}

VulkanNode::~VulkanNode()
{
    //FIXME: stall GPU
    vkDeviceWaitIdle(vulkanDevice->vkDevice);
    uniform.Destroy();
    vulkanDevice = nullptr;
    transform = nullptr;
}

} // namespace renderer
