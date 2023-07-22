#include "physics_context.h"

#include "dynamic_rigidbody.h"
#include "static_rigidbody.h"
#include "rigidbody.h"

#include "logger.h"

#include "component.h"
#include "components/dynamic_body_component.h"
#include "components/static_body_component.h"

namespace physics
{

void SimulationEventCallback::onTrigger(
	physx::PxTriggerPair* pairs, physx::PxU32 count)
{
	while(count--)
	{
		// if(pairs[i].flags & 
		// 	(physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER |
		// 	 physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
		// {
		// 	continue;
		// }

		const physx::PxTriggerPair& current = *pairs++;
		if(current.status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			// CollisionShape* shape =
			// 	static_cast<CollisionShape*>(current.otherShape->userData);
			
			/**
			 * shape->SetTriggerState(ENTER)
			 * shape->SetTriggerState(LEAVE)
			 * 
			 * shape->ProcessCallbacks()
			 * {
			 * 		if (ENTER)
			 * 		{
			 * 			// enter callback, go to stay state
			 * 		}
			 * 		
			 * }
			 * 
			 */
		}
		if(current.status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
		{

		}
	}
}

PhysicsContext::PhysicsContext(physx::PxPhysics* gPhysics)
{
	this->gPhysics = gPhysics;

    physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	sceneDesc.simulationEventCallback = &simulationEventCallback;
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
	physx::PxRigidStatic* body =
		gPhysics->createRigidStatic(physx::PxTransform(physx::PxIdentity));

	gScene->addActor(*body);

	StaticRigidbody* staticRigidBody = new StaticRigidbody(this, body);
	return staticRigidBody;
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
	GeometryType geometryType)
{
	physx::PxMaterial* material;
	physx::PxShape* shape;
	CollisionShape* collisionShape;

	switch (geometryType)
	{
	case GeometryType::eBOX:
		material = gPhysics->createMaterial(0.5f, 0.5f, 0.5f);
		shape = gPhysics->createShape(
			physx::PxBoxGeometry(0.5f, 0.5f, 0.5f),
			*material, true
		);
		collisionShape = new CollisionShape(shape);
		break;

	case GeometryType::eSPHERE:
		material = gPhysics->createMaterial(0.5f, 0.5f, 0.5f);
		shape = gPhysics->createShape(
			physx::PxSphereGeometry(0.5f),
			*material, true
		);
		collisionShape = new CollisionShape(shape);
		break;

	case GeometryType::eCAPSULE:
		material = gPhysics->createMaterial(0.5f, 0.5f, 0.5f);
		shape = gPhysics->createShape(
			physx::PxCapsuleGeometry(0.5f, 1.0f),
			*material, true
		);
		collisionShape = new CollisionShape(shape);
		break;

	// case GeometryType::ePLANE:
	// 	if (rigidbody->GetRigidbodyType() == RigidbodyType::eRIGID_STATIC)
	// 	{
	// 		shape = gPhysics->createShape(
	// 			physx::PxPlane(),
	// 			*material, true
	// 		);
	// 		physx::PxPlaneGeometry a;
		

	// 		collisionShape = new CollisionShape(shape);
	// 	}
	// 	else
	// 	{
	// 		Logger::Write(
	// 			"[Physics] Plane shape can only be added to static rigidbodies",
	// 			Logger::Level::Warning, Logger::MsgType::Physics
	// 		);
	// 		material->release();
	// 		return nullptr;
	// 	}
	// 	break;

	default:
		return nullptr;
		break;
	}

	return collisionShape;
}

void PhysicsContext::RemoveRigidbody(physx::PxRigidActor* actor)
{
	//gScene->removeActor(*actor);
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
		DynamicRigidbody* rigidbody = dynamic_cast<DynamicBodyComponent*>(
			e->GetComponent(Component::Type::DynamicBody))->dynamicBody;

		if (rigidbody->GetKinematic())
		{
			rigidbody->SetKinematicTarget(e->GetGlobalTransform());
		}
		else
		{
			rigidbody->SetGlobalTransform(e->GetGlobalTransform());
		}
	}
	else if (e->HasComponent(Component::Type::StaticBody))
	{
		StaticRigidbody* rigidbody = dynamic_cast<StaticBodyComponent*>(
			e->GetComponent(Component::Type::StaticBody))->staticBody;
	
		rigidbody->SetGlobalTransform(e->GetGlobalTransform());
	} 
} 

} // namespace physics
