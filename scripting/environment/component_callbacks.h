#pragma once

#include <v8-function-callback.h>
#include <v8-primitive.h>

namespace scripting
{

void GetEntity(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetComponentType(const v8::FunctionCallbackInfo<v8::Value> &info);
void SetMeshResoucePath(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetMeshResourcePath(const v8::FunctionCallbackInfo<v8::Value> &info);

} // namespace scripting
