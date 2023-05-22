#pragma once

#include <memory>
#include <glm/glm.hpp>


namespace renderer
{
    
struct Wire
{
    glm::vec3 color;
    float width;
    bool depthEnabled;
};

struct Line: public Wire
{
    glm::vec3 direciton;
    float length;
};

struct Sphere: public Wire
{
    glm::vec3 position;
    float radius;
};

struct Circle: public Wire
{
    glm::vec3 position;
    glm::vec3 normal;
    float radius;
};

struct AABB: public Wire
{
    glm::vec3 minCoordinates;
    glm::vec3 maxCoordinates;
};

struct OBB: public Wire
{
    glm::mat4 transform;
};

class Wireframe
{
    virtual std::shared_ptr<Line> GetLine(
        glm::vec3 direction, float length,
        glm::vec3 color = {0.0, 1.0, 0.0},
        float width = 1.0, bool depthEnabled = true) = 0;
    virtual std::shared_ptr<Sphere> GetSphere(
        glm::vec3 position, float radius,
        glm::vec3 color = {0.0, 1.0, 0.0}, 
        float width = 1.0, bool depthEnabled = true) = 0;
    virtual std::shared_ptr<Circle> GetCircle(
        glm::vec3 position, glm::vec3 normal, float radius,
        glm::vec3 color = {0.0, 1.0, 0.0},
        float width = 1.0, bool depthEnabled = true) = 0;
    virtual std::shared_ptr<AABB> GetAABB(
        glm::vec3 minCoordinates, glm::vec3 maxCoordinates,
        glm::vec3 color = {0.0, 1.0, 0.0},
        float width = 1.0, bool depthEnabled = true) = 0;
    virtual std::shared_ptr<OBB> GetOBB(
        glm::mat4 transform,
        glm::vec3 color = {0.0, 1.0, 0.0},
        float width = 1.0, bool depthEnabled = true) = 0;
};

} // namespace renderer
