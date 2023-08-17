#include "physics_system.h"

#include "physics_context.h"
#include "components/dynamic_body_component.h"
#include "components/static_body_component.h"

#include "component.h"

/**
 * PhysX is version 0x05010300 on Windows, 0x05010200 on Mac.
 * The header files are from the Windows version.
 * But APIs are pretty much the same.
 * Redefine the version here for Mac compatibility.
 * Just defer potential problems to the future me x_x??
*/
#ifdef __APPLE__
#undef PX_PHYSICS_VERSION
#define PX_PHYSICS_VERSION 0x05010200
#endif

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

    ComponentLocator::SetInitializer(Component::Type::DynamicBody,
        DynamicBodyInitializer(this));
    ComponentLocator::SetDeserializer(Component::Type::DynamicBody,
        DynamicBodyDeserializer(this));
    ComponentLocator::SetInitializer(Component::Type::StaticBody,
        StaticBodyInitializer(this));
    ComponentLocator::SetDeserializer(Component::Type::StaticBody,
        StaticBodyDeserializer(this));
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
