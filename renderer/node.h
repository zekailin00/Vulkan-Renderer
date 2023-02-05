#pragma once

#include "mesh.h"

#include <memory>
#include <vector>


namespace renderer
{

class Node
{
public:
    /**
     * Add a mesh to the current node.
     * If the node has already contained a mesh,
     * the old mesh will be returned;
     * otherwise, null pointer is returned.
    */
    std::shared_ptr<Mesh> AddMesh(std::shared_ptr<Mesh> mesh);

    /**
     * Add a child node to the current node.
     * If the node has already parented to another node,
     * return false.
    */
    bool AddChildNode(std::unique_ptr<Node> node);

    /**
     * Remove the child node.
     * When a child node is removed, its children are also removed,
     * so if each of the grandchildren only has only one reference,
     * they can all be freed.
     * If the node is not parented to the current node,
     * return false.
    */
    bool RemoveChildNode(Node* node);

    /**
     * Get the child node by its index in the list.
     * If the index goes out of bound,
     * it returns nullptr.
    */
    Node* GetChildNode(unsigned int index);

    // Prevent copy operation
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

private:
    std::shared_ptr<Mesh> mesh;

    // https://cplusplus.com/reference/memory/unique_ptr/get/
    // Node owns its children, and returns raw pointer to users.
    std::vector<std::unique_ptr<Node>> nodeLists;
};

} // namespace renderer