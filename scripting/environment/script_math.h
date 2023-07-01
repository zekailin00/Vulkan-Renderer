#pragma once

#include <v8-local-handle.h>
#include <v8-primitive.h>

#include <glm/mat4x4.hpp>

namespace scripting
{

v8::Local<v8::Object> toV8(
    const glm::vec2& vec2, v8::Isolate* isolate);

v8::Local<v8::Object> toV8(
    const glm::vec3& vec3, v8::Isolate* isolate);

v8::Local<v8::Object> toV8(
    const glm::vec4& vec4, v8::Isolate* isolate);

v8::Local<v8::Object> toV8(
    const glm::mat4& mat4, v8::Isolate* isolate);

bool toCpp(glm::vec2& vec2, v8::Local<v8::Object> v8Vec2,
    v8::Isolate* isolate);

bool toCpp(glm::vec3& vec3, v8::Local<v8::Object> v8Vec3,
    v8::Isolate* isolate);

bool toCpp(glm::vec4& vec4, v8::Local<v8::Object> v8Vec4,
    v8::Isolate* isolate);

bool toCpp(glm::mat4& mat4, v8::Local<v8::Object> v8Mat4,
    v8::Isolate* isolate);

namespace math
{

void Mat4Add(const v8::FunctionCallbackInfo<v8::Value> &info);
void Mat4Multiply(const v8::FunctionCallbackInfo<v8::Value> &info);
void Mat4Subtract(const v8::FunctionCallbackInfo<v8::Value> &info);
void Mat4Inverse(const v8::FunctionCallbackInfo<v8::Value> &info);
void Mat4Identity(const v8::FunctionCallbackInfo<v8::Value> &info);

/* EulerAngleXYZ */
void Mat4Rotation(const v8::FunctionCallbackInfo<v8::Value> &info);
void Mat4Translate(const v8::FunctionCallbackInfo<v8::Value> &info);
void Mat4Scale(const v8::FunctionCallbackInfo<v8::Value> &info);


} // namespace math

} // namespace scripting