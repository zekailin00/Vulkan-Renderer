#pragma once 

#include <v8-function.h>
#include <v8-local-handle.h>
#include <v8-template.h>
#include <v8-isolate.h>

namespace scripting
{

v8::Local<v8::ObjectTemplate> MakeSystemTemplate(v8::Isolate* isolate);
v8::Local<v8::FunctionTemplate> MakeEntityTemplate(v8::Isolate* isolate);
v8::Local<v8::FunctionTemplate> MakeSceneTemplate(v8::Isolate* isolate);

} // namespace scripting