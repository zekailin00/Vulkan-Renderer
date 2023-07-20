#include "physics_system.h"

#include "physics_context.h"
#include "components/dynamic_body_component.h"
#include "components/static_body_component.h"

#include "component.h"

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
