#pragma once

#include <v8-isolate.h>
#include <v8-local-handle.h>
#include <v8-persistent-handle.h>


namespace scripting
{

struct Templates
{
    static v8::Global<v8::ObjectTemplate> globalTemplate;
    static v8::Global<v8::ObjectTemplate> systemTemplate;
    static v8::Global<v8::ObjectTemplate> mathTemplate;
    static v8::Global<v8::ObjectTemplate> inputTemplate;
    static v8::Global<v8::ObjectTemplate> assetManagerTemplate;

    static v8::Global<v8::ObjectTemplate> meshCompTemplate;
    static v8::Global<v8::ObjectTemplate> lightCompTemplate;
    static v8::Global<v8::ObjectTemplate> dynamicBodyCompTemplate;
    static v8::Global<v8::ObjectTemplate> staticBodyCompTemplate;

    static v8::Global<v8::ObjectTemplate> collisionShapeTemplate;
    static v8::Global<v8::ObjectTemplate> physicsTemplate;

    static v8::Global<v8::FunctionTemplate> entityTemplate;
    static v8::Global<v8::FunctionTemplate> componentTemplate;
    static v8::Global<v8::FunctionTemplate> sceneTemplate;

    static void ResetAll()
    {
        globalTemplate.Reset();
        systemTemplate.Reset();
        mathTemplate.Reset();
        inputTemplate.Reset();
        assetManagerTemplate.Reset();

        meshCompTemplate.Reset();
        lightCompTemplate.Reset();
        dynamicBodyCompTemplate.Reset();
        staticBodyCompTemplate.Reset();

        collisionShapeTemplate.Reset();
        physicsTemplate.Reset();

        entityTemplate.Reset();
        componentTemplate.Reset();
        sceneTemplate.Reset();
    }
};

} // namespace scripting