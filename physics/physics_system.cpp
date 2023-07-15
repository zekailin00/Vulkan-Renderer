#include "physics_system.h"

#include "physics_context.h"

namespace physics
{

PhysicsSystem::PhysicsSystem()
{
    gFoundation = PxCreateFoundation(
        PX_PHYSICS_VERSION,
        gAllocator, gErrorCallback
    );

    gPvd = PxCreatePvd(*gFoundation);
	physx::PxPvdTransport* transport =
        physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	gPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(
        PX_PHYSICS_VERSION, *gFoundation,
        physx::PxTolerancesScale(), true, gPvd
    );
}

PhysicsSystem::~PhysicsSystem()
{
    PX_RELEASE(gPhysics);
    if(gPvd)
	{
		physx::PxPvdTransport* transport = gPvd->getTransport();
		gPvd->release();
        gPvd = NULL;
		PX_RELEASE(transport);
	}
	PX_RELEASE(gFoundation);
}

PhysicsContext* PhysicsSystem::NewContext()
{
    return new PhysicsContext(gPhysics);
}


} // namespace physic
