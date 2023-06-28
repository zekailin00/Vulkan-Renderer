#pragma once

#include <v8-local-handle.h>
#include <v8-primitive.h>

#include <glm/mat4x4.hpp>

namespace scripting
{

v8::Local<v8::Object> toV8(
    const glm::vec2& vec2, const v8::FunctionCallbackInfo<v8::Value> &info);

v8::Local<v8::Object> toV8(
    const glm::vec3& vec3, const v8::FunctionCallbackInfo<v8::Value> &info);

v8::Local<v8::Object> toV8(
    const glm::vec4& vec4, const v8::FunctionCallbackInfo<v8::Value> &info);

v8::Local<v8::Object> toV8(
    const glm::mat4& mat4, const v8::FunctionCallbackInfo<v8::Value> &info);

bool toCpp(glm::vec2& vec2, v8::Local<v8::Object> v8Vec2,
    const v8::FunctionCallbackInfo<v8::Value> &info);

bool toCpp(glm::vec3& vec3, v8::Local<v8::Object> v8Vec3,
    const v8::FunctionCallbackInfo<v8::Value> &info);

bool toCpp(glm::vec4& vec4, v8::Local<v8::Object> v8Vec4,
    const v8::FunctionCallbackInfo<v8::Value> &info);

bool toCpp(glm::mat4& mat4, v8::Local<v8::Object> v8Mat4,
    const v8::FunctionCallbackInfo<v8::Value> &info);

} // namespace scripting