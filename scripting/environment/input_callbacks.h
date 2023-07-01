#pragma once

#include <v8-local-handle.h>
#include <v8-primitive.h>


namespace scripting
{

/**
 *  System.Input.LeftAimEvent((mat4) => {
 *      entity.SetLocalTransform1(mat4)
 *  })
 */


void LeftAimEvent(const v8::FunctionCallbackInfo<v8::Value> &info);
void RightAimEvent(const v8::FunctionCallbackInfo<v8::Value> &info);
void LeftGripEvent(const v8::FunctionCallbackInfo<v8::Value> &info);
void RightGripEvent(const v8::FunctionCallbackInfo<v8::Value> &info);
void ControllerEvent(const v8::FunctionCallbackInfo<v8::Value> &info);

} // namespace scripting
