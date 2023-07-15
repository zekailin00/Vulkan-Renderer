#pragma once

#include <PxPhysicsAPI.h>

namespace physics
{

class PhysicsContext;

class PhysicsSystem
{
public:
    PhysicsSystem();

    ~PhysicsSystem();

    PhysicsContext* NewContext();

private:
    physx::PxFoundation* gFoundation;
    physx::PxPvd*        gPvd;
    physx::PxPhysics*    gPhysics;

    physx::PxDefaultAllocator		gAllocator{};
    physx::PxDefaultErrorCallback	gErrorCallback{};
};


} // namespace physics