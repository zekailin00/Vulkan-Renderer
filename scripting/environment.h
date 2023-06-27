#pragma once 

#include <v8-function.h>
#include <v8-local-handle.h>
#include <v8-template.h>
#include <v8-isolate.h>

#include "entity.h"

v8::Local<v8::ObjectTemplate> MakeSystemTemplate(v8::Isolate* isolate)
{
    return v8::ObjectTemplate::New(isolate);
}

v8::Local<v8::ObjectTemplate> MakeEntityTemplate(v8::Isolate* isolate)
{
    return v8::ObjectTemplate::New(isolate);
}

v8::Local<v8::ObjectTemplate> MakeSceneTemplate(v8::Isolate* isolate)
{
    return v8::ObjectTemplate::New(isolate);
}
v8::Local<v8::ObjectTemplate> MakeConsoleTemplate(v8::Isolate* isolate)
{
    return v8::ObjectTemplate::New(isolate);
}