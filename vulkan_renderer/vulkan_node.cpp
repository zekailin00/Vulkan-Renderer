#include "vulkan_node.h"

#include <memory>
#include <utility> // std::move

namespace renderer
{

std::shared_ptr<Mesh> VulkanNode::AddMesh(std::shared_ptr<Mesh> mesh)
{
    this->mesh = mesh;
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
