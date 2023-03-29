#include "mesh.h"
#include "objloader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#include <string>
#include <vector>

namespace renderer
{

void ObjLoader(std::string& meshPath,
    std::vector<renderer::Vertex>& modelVertices,
    std::vector<unsigned int>& modelIndices)
{
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning, error;

    if (!tinyobj::LoadObj(&attributes, &shapes, &materials,
        &warning, &error, meshPath.c_str()))
    {
        throw;
    }

    for (tinyobj::shape_t shape : shapes) {
        for (const tinyobj::index_t& index : shape.mesh.indices) {
            renderer::Vertex vertex{};

            int vertIndex = index.vertex_index;
            int normIndex = index.normal_index;
            int texIndex = index.texcoord_index; 

            vertex.Position = {
                attributes.vertices[vertIndex * 3 + 0],
                attributes.vertices[vertIndex * 3 + 1],
                attributes.vertices[vertIndex * 3 + 2],
            };

            vertex.Normal = {
                attributes.normals[normIndex * 3 + 0],
                attributes.normals[normIndex * 3 + 1],
                attributes.normals[normIndex * 3 + 2],
            };


            if (texIndex < 0)
            {
                vertex.TexCoords = {0, 0};
            } else {
                vertex.TexCoords = {
                    attributes.texcoords[texIndex * 2 + 0],
                    1 - attributes.texcoords[texIndex * 2 + 1],
                };
            }

            modelVertices.push_back(vertex);
            modelIndices.push_back(static_cast<unsigned int>(modelIndices.size()));
        }
    }
}

} // namespace renderer