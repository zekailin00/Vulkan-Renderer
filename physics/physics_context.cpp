#include "physics_context.h"

#include "dynamic_rigidbody.h"
#include "static_rigidbody.h"


namespace physics
{

PhysicsContext::PhysicsContext(physx::PxPhysics* gPhysics)
{
	this->gPhysics = gPhysics;

    physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher	= gDispatcher;
	sceneDesc.filterShader	= physx::PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

    physx::PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if(pvdClient)
	{
		pvdClient->setScenePvdFlag(
            physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(
            physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(
            physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
}

PhysicsContext::~PhysicsContext()
{
    PX_RELEASE(gScene);
	PX_RELEASE(gDispatcher);
	gPhysics = nullptr;
}

StaticRigidbody* PhysicsContext::NewStaticRigidbody()
{
	return nullptr;
}

DynamicRigidbody* PhysicsContext::NewDynamicRigidbody()
{
	physx::PxRigidDynamic* body =
		gPhysics->createRigidDynamic(physx::PxTransform(physx::PxIdentity));

	gScene->addActor(*body);

	DynamicRigidbody* dynamicRigidBody = new DynamicRigidbody(body);
	return dynamicRigidBody;
}

CollisionShape* PhysicsContext::AddCollisionShape(
	DynamicRigidbody* rigidbody,
	CollisionShape::Geometry geometry)
{
	CollisionShape* collisionShape;
	physx::PxShape* shape;
	physx::PxMaterial* material = gPhysics->createMaterial(
		0.5f, 0.5f, 0.5f
	);

	switch (geometry)
	{
	case CollisionShape::Geometry::Box:
		shape = gPhysics->createShape(
			physx::PxBoxGeometry(0.5f, 0.5f, 0.5f),
			*material, true
		);
		collisionShape = new CollisionShape(shape);
		break;

	case CollisionShape::Geometry::Sphere:
		shape = gPhysics->createShape(
			physx::PxSphereGeometry(0.5f),
			*material, true
		);
		collisionShape = new CollisionShape(shape);
		break;

	case CollisionShape::Geometry::Capsule:
		shape = gPhysics->createShape(
			physx::PxCapsuleGeometry(0.5f, 1.0f),
			*material, true
		);
		collisionShape = new CollisionShape(shape);
		break;

	case CollisionShape::Geometry::Plane:
		// Free material and others
		material->release();
		return nullptr;
		break;

	default:
		throw;
	}

	rigidbody->AttachShape(collisionShape);
	return collisionShape;
}

} // namespace physics
