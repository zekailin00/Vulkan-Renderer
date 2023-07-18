#include "physics_context.h"

#include "dynamic_rigidbody.h"
#include "static_rigidbody.h"

#include "component.h"
#include "components/dynamic_body_component.h"

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

	DynamicRigidbody* dynamicRigidBody = new DynamicRigidbody(this, body);
	return dynamicRigidBody;
}

CollisionShape* PhysicsContext::AddCollisionShape(
	DynamicRigidbody* rigidbody,
	GeometryType geometryType)
{
	CollisionShape* collisionShape;
	physx::PxShape* shape;
	physx::PxMaterial* material = gPhysics->createMaterial(
		0.5f, 0.5f, 0.5f
	);

	switch (geometryType)
	{
	case GeometryType::eBOX:
		shape = gPhysics->createShape(
			physx::PxBoxGeometry(0.5f, 0.5f, 0.5f),
			*material, true
		);
		collisionShape = new CollisionShape(shape);
		break;

	case GeometryType::eSPHERE:
		shape = gPhysics->createShape(
			physx::PxSphereGeometry(0.5f),
			*material, true
		);
		collisionShape = new CollisionShape(shape);
		break;

	case GeometryType::eCAPSULE:
		shape = gPhysics->createShape(
			physx::PxCapsuleGeometry(0.5f, 1.0f),
			*material, true
		);
		collisionShape = new CollisionShape(shape);
		break;

	default:
		material->release();
		return nullptr;
		break;
	}

	rigidbody->AttachShape(collisionShape);
	shape->release();
	return collisionShape;
}

int PhysicsContext::Simulate(Timestep ts)
{
	accumulator += ts;
    if(accumulator < STEP_SIZE)
        return 0;

	int simCount = 0;
	while (accumulator >= STEP_SIZE)
	{
		accumulator -= STEP_SIZE;
    	gScene->simulate(STEP_SIZE);
		gScene->fetchResults(true);
	}

    return simCount;
}

void PhysicsContext::UpdatePhysicsTransform(Entity* e)
{
	if (e->HasComponent(Component::Type::DynamicBody))
	{
		DynamicRigidbody * rigidbody = dynamic_cast<DynamicBodyComponent*>(
			e->GetComponent(Component::Type::DynamicBody))->dynamicBody;
		
		rigidbody->SetGlobalTransform(e->GetGlobalTransform());
		// kinematic objects have different methods to set tf
		
		// glm::vec3 scale = e->GetGlobalScale();
		// FIXME: SCALE ISSUE
		// scale->
	}
} 

} // namespace physics
