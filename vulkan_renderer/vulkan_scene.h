#pragma once 

#include "scene.h"

namespace renderer
{

class VulkanScene: public Scene
{
public:
    Node* GetRootNode() override;

    VulkanScene();
    ~VulkanScene() override;
};

} // namespace renderer
