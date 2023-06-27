#pragma once

#include <v8-function.h>
#include <v8-local-handle.h>
#include <v8-template.h>
#include <v8-isolate.h>

v8::Local<v8::ObjectTemplate> MakeIdentifierTemplate(v8::Isolate* isolate);
v8::Local<v8::ObjectTemplate> MakeTransformTemplate(v8::Isolate* isolate);
v8::Local<v8::ObjectTemplate> MakeHierarchyTemplate(v8::Isolate* isolate);
v8::Local<v8::ObjectTemplate> MakeJavascriptTemplate(v8::Isolate* isolate);

v8::Local<v8::ObjectTemplate> MakeMeshTemplate(v8::Isolate* isolate);
v8::Local<v8::ObjectTemplate> MakeCameraTemplate(v8::Isolate* isolate);
v8::Local<v8::ObjectTemplate> MakeLightTemplate(v8::Isolate* isolate);