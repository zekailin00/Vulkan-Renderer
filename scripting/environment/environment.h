#pragma once 

#include <v8-function.h>
#include <v8-local-handle.h>
#include <v8-template.h>
#include <v8-isolate.h>

namespace scripting
{
    
v8::Local<v8::FunctionTemplate> MakeEntityTemplate(v8::Isolate* isolate);
v8::Local<v8::FunctionTemplate> MakeSceneTemplate(v8::Isolate* isolate);
v8::Local<v8::ObjectTemplate> MakeMathTemplate(v8::Isolate* isolate);
v8::Local<v8::ObjectTemplate> MakeInputTemplate(v8::Isolate* isolate);
v8::Local<v8::ObjectTemplate> MakeAssetManagerTemplate(v8::Isolate* isolate);

v8::Local<v8::ObjectTemplate> MakeMeshCompTemplate(v8::Isolate* isolate);
v8::Local<v8::ObjectTemplate> MakeLightCompTemplate(v8::Isolate* isolate);
v8::Local<v8::ObjectTemplate> MakeDynamicBodyCompTemplate(v8::Isolate* isolate);

v8::Local<v8::ObjectTemplate> MakeCollsionShapeTemplate(v8::Isolate* isolate);

void print(const v8::FunctionCallbackInfo<v8::Value>& args);

} // namespace scripting