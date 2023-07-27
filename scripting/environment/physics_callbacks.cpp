#include "physics_callbacks.h"

#include <v8-external.h>
#include <v8-object.h>
#include <v8-template.h>
#include <v8-function.h>
#include <v8-local-handle.h>
#include <v8-primitive.h>

#include "script.h"
#include "script_context.h"
#include "script_exception.h"

#include "environment_templates.h"
#include "script_math.h"

#include "collision_shape.h"
#include "logger.h"
#include "validation.h"


namespace scripting
{

class ScriptTriggerCallback
{
    Script::TriggerContext* triggerContext;
public:
    ScriptTriggerCallback(Script::TriggerContext* triggerContext):
        triggerContext(triggerContext) {}

    void operator() (physics::TriggerEvent* event)
    {
        //FIXME: all callbacks do NOT have environment setup from script.cpp
        v8::HandleScope handleScope(triggerContext->isolate);
        v8::Local<v8::Context> localContext = v8::Local<v8::Context>::New(
            triggerContext->isolate,
            triggerContext->scriptContext->GetContext()
        );
        v8::Context::Scope contextScope(localContext);

        v8::Local<v8::Object> v8Event = v8::Object::New(triggerContext->isolate);

        v8::Local<v8::Value> keyTriggerEntity =
            v8::String::NewFromUtf8Literal(triggerContext->isolate, "triggerEntity");
        v8::Local<v8::Value> keyTriggerCollisionShape =
            v8::String::NewFromUtf8Literal(triggerContext->isolate, "triggerCollisionShape");
        v8::Local<v8::Value> keyOtherEntity =
            v8::String::NewFromUtf8Literal(triggerContext->isolate, "otherEntity");
        v8::Local<v8::Value> keyOtherCollisionShape =
            v8::String::NewFromUtf8Literal(triggerContext->isolate, "otherCollisionShape");

        v8::Local<v8::FunctionTemplate> entityTemplate =
            v8::Local<v8::FunctionTemplate>::New(
                triggerContext->isolate, Templates::entityTemplate);
        v8::Local<v8::Function> entityFunction =
            entityTemplate->GetFunction(localContext).ToLocalChecked();

        v8::Local<v8::Object> v8TriggerEntity = 
            entityFunction->NewInstance(localContext).ToLocalChecked();
        v8TriggerEntity->SetInternalField(0,
            v8::External::New(triggerContext->isolate, event->triggerEntity));
        v8::Local<v8::Object> v8OtherEntity = 
            entityFunction->NewInstance(localContext).ToLocalChecked();
        v8OtherEntity->SetInternalField(0,
            v8::External::New(triggerContext->isolate, event->otherEntity));

        v8::Local<v8::ObjectTemplate> v8CollisionShapeTemp =
        v8::Local<v8::ObjectTemplate>::New(
            triggerContext->isolate, Templates::collisionShapeTemplate);

        v8::Local<v8::Object> v8TriggerCollisionShape =
            v8CollisionShapeTemp->NewInstance(localContext).ToLocalChecked();
        v8TriggerCollisionShape->SetInternalField(0,
            v8::External::New(triggerContext->isolate, event->triggerCollisionShape));
        v8::Local<v8::Object> v8OtherCollisionShape =
            v8CollisionShapeTemp->NewInstance(localContext).ToLocalChecked();
        v8OtherCollisionShape->SetInternalField(0,
            v8::External::New(triggerContext->isolate, event->otherCollisionShape));

        v8::Local<v8::Function> callback = v8::Local<v8::Function>::New(
                triggerContext->isolate,
                triggerContext->function
        );

        v8::TryCatch tryCatch(triggerContext->isolate);
        v8::Local<v8::Value> result;

        if (!callback->Call(localContext, localContext->Global(), 1, &v8Event.As<v8::Value>())
            .ToLocal(&result))
        {
            ASSERT(tryCatch.HasCaught());
            ExceptionHandler(&tryCatch, triggerContext->isolate);
        }
        
    }
};

void CollisionShape::SetLocalTransform(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());
    v8::Isolate* isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    // FIXME: referencing a pointer that has already been released.
    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);
    
    if (info.Length() != 1 || !info[0]->IsObject())
    {
        Logger::Write(
            "[Scripting] SetLocalTransform parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    glm::mat4 transform;
    if (toCpp(transform, info[0].As<v8::Object>(), isolate))
    {
        collisionShape->SetLocalTransform(transform);
    }
    else
    {
        Logger::Write(
            "[Scripting] SetLocalTransform parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
    }
}

void CollisionShape::GetLocalTransform(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());
    v8::Isolate* isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    glm::mat4 transform;
    collisionShape->GetLocalTransform(transform);
    v8::Local<v8::Object> v8Transform = toV8(transform, isolate);
    info.GetReturnValue().Set(v8Transform);
}

void CollisionShape::SetTrigger(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());
    v8::Isolate* isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    if (info.Length() != 1 || !info[0]->IsBoolean())
    {
        Logger::Write(
            "[Scripting] SetTrigger parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    bool isTrigger = info[0]->BooleanValue(isolate);
    collisionShape->SetTrigger(isTrigger);
}

void CollisionShape::GetTrigger(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());
    v8::Isolate* isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    bool isTrigger = collisionShape->GetTrigger();
    info.GetReturnValue().Set(isTrigger);
}

void CollisionShape::SetOnTriggerEnter(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    if (info.Length() != 1 || !info[0]->IsFunction())
    {
        Logger::Write(
            "[Scripting] SetOnTriggerEnter parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );

        return;
    }

    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    v8::Local<v8::Value> keySystem =
        v8::String::NewFromUtf8Literal(isolate, "System");
    v8::Local<v8::Value> keyInternal =
        v8::String::NewFromUtf8Literal(isolate, "Internal");

    v8::Local<v8::Object> v8Internal = context->Global()
        ->Get(context, keySystem).ToLocalChecked().As<v8::Object>()
        ->Get(context, keyInternal).ToLocalChecked().As<v8::Object>();

    Script* script = static_cast<Script*>(
        v8Internal->GetInternalField(0).As<v8::External>()->Value()
    );
    
    Script::TriggerContext* triggerContext = new Script::TriggerContext();
    triggerContext->function.Reset(isolate, info[0].As<v8::Function>());
    triggerContext->collisionShape = collisionShape;
    collisionShape->SetOnTriggerEnter(ScriptTriggerCallback(triggerContext));
    script->AddTriggerContext(triggerContext);
}

void CollisionShape::SetOnTriggerStay(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    if (info.Length() != 1 || !info[0]->IsFunction())
    {
        Logger::Write(
            "[Scripting] SetOnTriggerEnter parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );

        return;
    }

    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    v8::Local<v8::Value> keySystem =
        v8::String::NewFromUtf8Literal(isolate, "System");
    v8::Local<v8::Value> keyInternal =
        v8::String::NewFromUtf8Literal(isolate, "Internal");

    v8::Local<v8::Object> v8Internal = context->Global()
        ->Get(context, keySystem).ToLocalChecked().As<v8::Object>()
        ->Get(context, keyInternal).ToLocalChecked().As<v8::Object>();

    Script* script = static_cast<Script*>(
        v8Internal->GetInternalField(0).As<v8::External>()->Value()
    );
    
    Script::TriggerContext* triggerContext = new Script::TriggerContext();
    triggerContext->function.Reset(isolate, info[0].As<v8::Function>());
    triggerContext->collisionShape = collisionShape;
    collisionShape->SetOnTriggerEnter(ScriptTriggerCallback(triggerContext));
    script->AddTriggerContext(triggerContext);
}

void CollisionShape::SetOnTriggerLeave(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{

}

void CollisionShape::ClearOnTriggerEnter(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{

}

void CollisionShape::ClearOnTriggerStay(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{

}

void CollisionShape::ClearOnTriggerLeave(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{

}

void CollisionShape::GetGeometryType(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());
    v8::Isolate* isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    physics::GeometryType type = collisionShape->GetGeometryType();
    info.GetReturnValue().Set(type);
}

void CollisionShape::SetGeometry(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());
    v8::Isolate* isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    if (info.Length() != 1 || !info[0]->IsObject())
    {
        Logger::Write(
            "[Scripting] SetGeometry parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    v8::Local<v8::Object> v8Object = info[0].As<v8::Object>();
    v8::Local<v8::Value> key;

    v8::Local<v8::Value> v8GeometryType;
    key = v8::String::NewFromUtf8Literal(isolate, "GeometryType");
    if (!v8Object->Get(context, key).ToLocal(&v8GeometryType) ||
        !v8GeometryType->IsInt32())
    {
        Logger::Write(
            "[Scripting] SetGeometry parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    physics::GeometryType geometryType = static_cast<physics::GeometryType>(
        v8GeometryType->Int32Value(context).ToChecked());

    //FIXME: C++ segfaults if JS keys are modified
    switch (geometryType)
    {
    case physics::GeometryType::eBOX:
    {
        key = v8::String::NewFromUtf8Literal(isolate, "halfExtent");
        v8::Local<v8::Object> v8HalfExtent =
            v8Object->Get(context, key).ToLocalChecked().As<v8::Object>();

        glm::vec3 halfExtent;
        ASSERT(toCpp(halfExtent, v8HalfExtent, isolate));
        physics::BoxGeometry box(
            halfExtent.x,
            halfExtent.y,
            halfExtent.z
        );
        collisionShape->SetGeometry(box);
    }
        break;
    case physics::GeometryType::eSPHERE:
    {
        key = v8::String::NewFromUtf8Literal(isolate, "radius");
        v8::Local<v8::Number> v8Radius =
            v8Object->Get(context, key).ToLocalChecked().As<v8::Number>();
        float radius = v8Radius->NumberValue(context).ToChecked();

        physics::SphereGeometry sphere(radius);
        collisionShape->SetGeometry(sphere);
    }
        break;
    case physics::GeometryType::eCAPSULE:
    {
        key = v8::String::NewFromUtf8Literal(isolate, "radius");
        v8::Local<v8::Number> v8Radius =
            v8Object->Get(context, key).ToLocalChecked().As<v8::Number>();
        float radius = v8Radius->NumberValue(context).ToChecked();

        key = v8::String::NewFromUtf8Literal(isolate, "halfHeight");
        v8::Local<v8::Number> v8HalfHeight =
            v8Object->Get(context, key).ToLocalChecked().As<v8::Number>();
        float halfHeight = v8HalfHeight->NumberValue(context).ToChecked();

        physics::CapsuleGeometry capsule(radius, halfHeight);
        collisionShape->SetGeometry(capsule);
    }
        break;
    
    default:
        Logger::Write(
            "[Scripting] SetGeometry parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        break;
    }

    return;
}

void CollisionShape::GetBoxGeometry(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());
    v8::Isolate* isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    physics::BoxGeometry box;
    if (!collisionShape->GetBoxGeometry(box))
    {
        Logger::Write(
            "[Scripting] GetBoxGeometry failed due to type mismatch",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    v8::Local<v8::Value> keySystem =
        v8::String::NewFromUtf8Literal(isolate, "System");
    v8::Local<v8::Value> keyPhysics =
        v8::String::NewFromUtf8Literal(isolate, "Physics");
    v8::Local<v8::Value> keyGeometry =
        v8::String::NewFromUtf8Literal(isolate, "BoxGeometry");

    v8::Local<v8::Function> constructor = context->Global()
        ->Get(context, keySystem).ToLocalChecked().As<v8::Object>()
        ->Get(context, keyPhysics).ToLocalChecked().As<v8::Object>()
        ->Get(context, keyGeometry).ToLocalChecked().As<v8::Function>();

    v8::Local<v8::Value> argv[3] {
        v8::Int32::New(isolate, box.halfExtents.x),
        v8::Int32::New(isolate, box.halfExtents.y),
        v8::Int32::New(isolate, box.halfExtents.z)
    };

    v8::Local<v8::Object> v8Box =
        constructor->NewInstance(context, 3, argv).ToLocalChecked();
    info.GetReturnValue().Set(v8Box);
}

void CollisionShape::GetSphereGeometry(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());
    v8::Isolate* isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    physics::SphereGeometry sphere;
    if (!collisionShape->GetSphereGeometry(sphere))
    {
        Logger::Write(
            "[Scripting] GetSphereGeometry failed due to type mismatch",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    v8::Local<v8::Value> keySystem =
        v8::String::NewFromUtf8Literal(isolate, "System");
    v8::Local<v8::Value> keyPhysics =
        v8::String::NewFromUtf8Literal(isolate, "Physics");
    v8::Local<v8::Value> keyGeometry =
        v8::String::NewFromUtf8Literal(isolate, "SphereGeometry");

    v8::Local<v8::Function> constructor = context->Global()
        ->Get(context, keySystem).ToLocalChecked().As<v8::Object>()
        ->Get(context, keyPhysics).ToLocalChecked().As<v8::Object>()
        ->Get(context, keyGeometry).ToLocalChecked().As<v8::Function>();

    v8::Local<v8::Value> argv[1] {v8::Number::New(isolate, sphere.radius)};

    v8::Local<v8::Object> v8Sphere =
        constructor->NewInstance(context, 1, argv).ToLocalChecked();
    info.GetReturnValue().Set(v8Sphere);
}

void CollisionShape::GetCapsuleGeometry(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());
    v8::Isolate* isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    physics::CapsuleGeometry capsule;
    if (!collisionShape->GetCapsuleGeometry(capsule))
    {
        Logger::Write(
            "[Scripting] GetCapsuleGeometry failed due to type mismatch",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    v8::Local<v8::Value> keySystem =
        v8::String::NewFromUtf8Literal(isolate, "System");
    v8::Local<v8::Value> keyPhysics =
        v8::String::NewFromUtf8Literal(isolate, "Physics");
    v8::Local<v8::Value> keyGeometry =
        v8::String::NewFromUtf8Literal(isolate, "CapsuleGeometry");

    v8::Local<v8::Function> constructor = context->Global()
        ->Get(context, keySystem).ToLocalChecked().As<v8::Object>()
        ->Get(context, keyPhysics).ToLocalChecked().As<v8::Object>()
        ->Get(context, keyGeometry).ToLocalChecked().As<v8::Function>();

    v8::Local<v8::Value> argv[2] {
        v8::Number::New(isolate, capsule.halfHeight),
        v8::Number::New(isolate, capsule.radius)
    };

    v8::Local<v8::Object> v8Capsule =
        constructor->NewInstance(context, 2, argv).ToLocalChecked();
    info.GetReturnValue().Set(v8Capsule);
}

void CollisionShape::GetPlaneGeometry(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());
    v8::Isolate* isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    physics::PlaneGeometry plane;
    if (!collisionShape->GetPlaneGeometry(plane))
    {
        Logger::Write(
            "[Scripting] GetPlaneGeometry failed due to type mismatch",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    return; //TODO: not supported yet
}

void CollisionShape::SetDynamicFriction(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());
    v8::Isolate* isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    if (info.Length() != 1 || !info[0]->IsNumber())
    {
        Logger::Write(
            "[Scripting] SetDynamicFriction parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    float dynamicFriction = info[0]->NumberValue(context).ToChecked();
    collisionShape->SetDynamicFriction(dynamicFriction);
}

void CollisionShape::GetDynamicFriction(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    float dynamicFriction = collisionShape->GetDynamicFriction();
    info.GetReturnValue().Set(dynamicFriction);
}

void CollisionShape::SetStaticFriction(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());
    v8::Isolate* isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    if (info.Length() != 1 || !info[0]->IsNumber())
    {
        Logger::Write(
            "[Scripting] SetStaticFriction parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    float staticFriction = info[0]->NumberValue(context).ToChecked();
    collisionShape->SetStaticFriction(staticFriction);
}

void CollisionShape::GetStaticFriction(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    float staticFriction = collisionShape->GetStaticFriction();
    info.GetReturnValue().Set(staticFriction);
}

void CollisionShape::SetRestitution(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());
    v8::Isolate* isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    if (info.Length() != 1 || !info[0]->IsNumber())
    {
        Logger::Write(
            "[Scripting] SetRestitution parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    float restitution = info[0]->NumberValue(context).ToChecked();
    collisionShape->SetRestitution(restitution);
}

void CollisionShape::GetRestitution(
    const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::CollisionShape* collisionShape =
        static_cast<physics::CollisionShape*>(field->Value());
    ASSERT(collisionShape != nullptr);

    float restitution = collisionShape->GetRestitution();
    info.GetReturnValue().Set(restitution);
}

} // namespace scripting
