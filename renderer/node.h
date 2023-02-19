#pragma once

#include "mesh.h"
#include "light.h"
#include "camera.h"

#include <memory>
#include <list>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>


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
    virtual std::shared_ptr<Mesh> AddMesh(std::shared_ptr<Mesh> mesh) = 0;

    /**
     * Add a child node to the current node and return its pointer as handle.
     * If the node has already parented to another node,
     * return nullptr.
    */
    virtual Node* AddChildNode(std::unique_ptr<Node> node) = 0;

    /**
     * Remove the child node.
     * When a child node is removed, its children are also removed,
     * so if each of the grandchildren only has only one reference,
     * they can all be freed.
     * If the node is not parented to the current node,
     * return false.
    */
    virtual std::unique_ptr<Node> RemoveChildNode(Node* node) = 0;

    /**
     * Get the child node by its index in the list.
     * If the index goes out of bound,
     * it returns nullptr.
    */
    virtual Node* GetChildNode(unsigned int index) = 0;

    virtual glm::mat4 GetTransform() = 0;

    virtual void SetTransform(glm::mat4 transform) = 0;

    Node() = default;
    virtual ~Node() = default;

    // Prevent copy operation
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

protected:
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Light> light;
    std::shared_ptr<Camera> camera;

    // https://cplusplus.com/reference/memory/unique_ptr/get/
    // Node owns its children, and returns raw pointer to users.
    std::list<std::unique_ptr<Node>> nodeLists;
};

} // namespace renderer