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
    Node* GetRootNode();

    // TODO: create new unique pointer.
    Scene();

    // TODO: Recursively remove nodes.
    ~Scene();

    // Prevent copy operation.
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

private:
    std::unique_ptr<Node> rootNode;
};
    
} // namespace renderer