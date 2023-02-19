#pragma once 

#include "material.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <string>

namespace renderer
{

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct BuildMeshInfo
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

class Mesh
{

public:

    /**
     * Add a material to the mesh.
     * If no material is added, the default material is used.
     * If a material is already added,
     * the old one will be replaced by the new material.
     * The old material will be deallocated 
     * if this is the only reference to the material.
    */
    virtual void AddMaterial(std::shared_ptr<Material> material) = 0;

    /**
     * Remove the current material,
     * and apply the default material to the mesh.
    */
    virtual void RemoveMaterial() = 0;

    Mesh() = default;
    virtual ~Mesh() = default;

    // Prevent copy operation.
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

protected:
    std::shared_ptr<Material> material;
};

} // namespace renderer