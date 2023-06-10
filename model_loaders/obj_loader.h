#pragma once

#include <vector>
#include <mesh.h>

bool ObjLoader2(std::string& meshPath,
    std::vector<renderer::Vertex>& modelVertices,
    std::vector<unsigned int>& modelIndices);
