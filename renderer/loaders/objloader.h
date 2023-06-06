#pragma once


namespace renderer
{

void ObjLoader(std::string& meshPath,
    std::vector<renderer::Vertex>& modelVertices,
    std::vector<unsigned int>& modelIndices);

} // namespace renderer