#pragma once

#include <v8-function.h>

namespace scripting
{

void AddComponent(const v8::FunctionCallbackInfo<v8::Value> &info);
void RemoveComponent(const v8::FunctionCallbackInfo<v8::Value> &info);
void HasComponent(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetComponent(const v8::FunctionCallbackInfo<v8::Value> &info);
void ReparentTo(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetGlobalTransform(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetLocalTransform(const v8::FunctionCallbackInfo<v8::Value> &info);
void SetLocalTransform1(const v8::FunctionCallbackInfo<v8::Value> &info);
void SetLocalTransform2(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetLocalTranslation(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetLocalRotation(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetLocalScale(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetParent(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetChildByName(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetChildren(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetName(const v8::FunctionCallbackInfo<v8::Value> &info);
void SetName(const v8::FunctionCallbackInfo<v8::Value> &info);
void GetScene(const v8::FunctionCallbackInfo<v8::Value> &info);

} // namespace scripting