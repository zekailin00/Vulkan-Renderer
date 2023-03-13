#include "vulkan_scene.h"

#include "vulkan_node.h"

#include <memory>


namespace renderer
{

Node* VulkanScene::GetRootNode()
{
    if (this->rootNode == nullptr)
        throw;
    return &(*this->rootNode);
}

VulkanScene::VulkanScene()
{
    this->rootNode = std::make_unique<VulkanNode>();
}

VulkanScene::~VulkanScene()
{
    /* No vulkan resources allocated. Do nothing. */
}

} // namespace renderer
