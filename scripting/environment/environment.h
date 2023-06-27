#pragma once 

#include <v8-function.h>
#include <v8-local-handle.h>
#include <v8-template.h>
#include <v8-isolate.h>

v8::Local<v8::ObjectTemplate> MakeSystemTemplate(v8::Isolate* isolate);