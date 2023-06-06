#pragma once 

#include <glm/glm.hpp>

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

typedef uint32_t VertexIndex;

struct MeshProperties
{
    glm::vec3 color;
    glm::vec3 specular; /* Not used */
};

struct MeshCoordinates
{
    glm::mat4 model;
};

struct LightProperties
{
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
};