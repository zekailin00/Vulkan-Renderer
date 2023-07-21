#pragma once

#include <v8-function-callback.h>
#include <v8-primitive.h>

namespace scripting
{

void GetEntity(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetComponentType(const v8::FunctionCallbackInfo<v8::Value> &info);

void SetMeshResourcePath(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetMeshResourcePath(const v8::FunctionCallbackInfo<v8::Value> &info);

void SetLightColor(const v8::FunctionCallbackInfo<v8::Value> &info);

class DynamicRigidbody
{
public:
    static void AttachShape(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void DetachShape(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetNbShapes(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetShapes(const v8::FunctionCallbackInfo<v8::Value> &info);

    static void SetGravity(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetGravity(const v8::FunctionCallbackInfo<v8::Value> &info);
 
    static void WakeUp(const v8::FunctionCallbackInfo<v8::Value> &info);
 
    static void SetDensity(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetDensity(const v8::FunctionCallbackInfo<v8::Value> &info);
 
    // static void SetMass(float mass);
    static void GetMass(const v8::FunctionCallbackInfo<v8::Value> &info); 
    // static void SetMassSpaceInertiaTensor(const glm::vec3& inertia);
    static void GetMassSpaceInertiaTensor(
        const v8::FunctionCallbackInfo<v8::Value> &info);
 
    static void SetLinearDamping(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetLinearDamping(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void SetAngularDamping(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetAngularDamping(const v8::FunctionCallbackInfo<v8::Value> &info);
 
    static void AddForce(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void AddTorque(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void ClearForce(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void ClearTorque(const v8::FunctionCallbackInfo<v8::Value> &info);
 
    static void GetLinearVelocity(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void SetLinearVelocity(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetAngularVelocity(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void SetAngularVelocity(const v8::FunctionCallbackInfo<v8::Value> &info);
 
    static void SetKinematic(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetKinematic(const v8::FunctionCallbackInfo<v8::Value> &info);
    // static void SetKinematicTarget(const glm::mat4& destination);

};

} // namespace scripting
