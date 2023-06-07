#pragma once

#include <memory>
#include <glm/glm.hpp>


namespace renderer
{

class Node; 

class Wireframe
{
public:
    enum WireframeType
    {
        Line,
        OBB
    };
public:
    virtual WireframeType GetType() = 0;
};

} // namespace renderer
