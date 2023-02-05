#pragma once 

#include "material.h"

#include <glm/glm.hpp>

#include <memory>
#include <string>

namespace renderer
{

struct Vertex
{
    glm::vec3 coord;
    glm::vec2 uv;
    glm::vec3 norm;
};

struct BuildMeshInfo
{
    Vertex* vertexData;
    int vertexCount;
    unsigned int* indexData;
    int indexCount;
};

class Mesh
{

public:

    /**
     * Build the material using the supplied mesh and indices.
     * If mesh cannot be built, false is returned;
     * BuildMesh can be called multiple time.
     * The old mesh will be removed when BuildMesh is called.
    */
    static std::shared_ptr<Mesh> BuildMesh(BuildMeshInfo& info);

    /**
     * Add a material to the mesh.
     * If no material is added, the default material is used.
     * If a material is already added,
     * the old one will be replaced by the new material.
     * The old material will be deallocated 
     * if this is the only reference to the material.
    */
    void AddMaterial(std::shared_ptr<Material> material);

    /**
     * Remove the current material,
     * and apply the default material to the mesh.
    */
    void RemoveMaterial();

    Mesh();
    ~Mesh();

    // Prevent copy operation.
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

private:
    std::shared_ptr<Material> material;
};

} // namespace renderer