#pragma once

#include <v8-function-callback.h>
#include <v8-primitive.h>


namespace scripting
{

class CollisionShape
{
public:
    static void SetLocalTransform(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetLocalTransform(const v8::FunctionCallbackInfo<v8::Value> &info);

    static void SetTrigger(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetTrigger(const v8::FunctionCallbackInfo<v8::Value> &info);

    static void GetGeometryType(const v8::FunctionCallbackInfo<v8::Value> &info);

    static void SetGeometry(const v8::FunctionCallbackInfo<v8::Value> &info);

    static void GetBoxGeometry(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetSphereGeometry(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetCapsuleGeometry(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetPlaneGeometry(const v8::FunctionCallbackInfo<v8::Value> &info);

    static void SetDynamicFriction(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetDynamicFriction(const v8::FunctionCallbackInfo<v8::Value> &info);

    static void SetStaticFriction(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetStaticFriction(const v8::FunctionCallbackInfo<v8::Value> &info);

    static void SetRestitution(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void GetRestitution(const v8::FunctionCallbackInfo<v8::Value> &info);
};

class Physics
{
public:
    static void RaycastClosest(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void Raycast(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void SweepClosest(const v8::FunctionCallbackInfo<v8::Value> &info);
    static void Sweep(const v8::FunctionCallbackInfo<v8::Value> &info);
};

} // namespace scripting
