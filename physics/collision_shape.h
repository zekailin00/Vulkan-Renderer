#pragma once

#include <PxPhysicsAPI.h>


namespace physics
{

class PhysicsContext;

class CollisionShape
{

public:
    enum class Geometry
    {
        Box,
        Sphere,
        Capsule,
        Plane
    };

private:
    CollisionShape(physx::PxShape* shape);

    friend PhysicsContext;

private:
    physx::PxShape* shape;
};

} // namespace physics
