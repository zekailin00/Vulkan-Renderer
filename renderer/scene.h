#pragma once 

#include "node.h"

#include <memory>

namespace renderer
{

class Scene
{

public:

    /**
     * Returns pointer to root node.
     * Root node is always owned by scene.
    */
    virtual Node* GetRootNode() = 0;

    Scene() = default;
    virtual ~Scene() = default;

    // Prevent copy operation.
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

protected:
    std::unique_ptr<Node> rootNode;
};
    
} // namespace renderer