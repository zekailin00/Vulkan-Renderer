#pragma once

#include <v8-function.h>
#include <v8-local-handle.h>
#include <v8-template.h>
#include <v8-isolate.h>


namespace scripting
{

void GetSceneName(const v8::FunctionCallbackInfo<v8::Value> &info);
void NewEntity(const v8::FunctionCallbackInfo<v8::Value> &info);
void RemoveEntity(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetEntityByName(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetEntitiesWithComponent(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetRootEntity(const v8::FunctionCallbackInfo<v8::Value> &info);

} // namespace scripting
