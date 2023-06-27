#include "component_templates.h"

#include <v8-primitive.h>
#include <v8-context.h>
#include <v8-external.h>
#include <v8-value.h>
#include <v8-container.h>

#include "experience.h"
#include "core_components.h"
#include "environment.h"

#include <string>


namespace JSCallbacks
{
    namespace Identifier
    {
        void GetName(v8::Local<v8::String> property,
            const v8::PropertyCallbackInfo<v8::Value>& info)
        {
            Entity entity = GetExternalEntity(info.Holder());
            std::string& name = entity.GetComponent<IdentifierComponent>().name;

            v8::Local<v8::String> nameStr =
                v8::String::NewFromUtf8(info.GetIsolate(), name.c_str())
                .ToLocalChecked();
            info.GetReturnValue().Set(nameStr);
        }

        void SetName(v8::Local<v8::String> property, v8::Local<v8::Value> value,
            const v8::PropertyCallbackInfo<void>& info)
        {
            Entity entity = GetExternalEntity(info.Holder());
            std::string& name = entity.GetComponent<IdentifierComponent>().name;

            v8::String::Utf8Value nameStr(info.GetIsolate(), value);
            name = *nameStr;
        }
    } // namespace Identifier

    namespace Hierarchy
    {
        void GetEntityByName(const v8::FunctionCallbackInfo<v8::Value>& args)
        {
            if (args.Length() < 1)
            return;

            v8::HandleScope handleScope(args.GetIsolate());

            Entity entity = GetExternalEntity(args.Holder());

            v8::Local<v8::Value> nameVal = args[0];
            v8::String::Utf8Value nameStr(args.GetIsolate(), nameVal);
            std::string name = *nameStr;

            EntityHierarchyComponent& hierarchy =
            entity.GetComponent<EntityHierarchyComponent>();
            
            Entity childEntity = hierarchy.GetChildByName(name);

            if (childEntity == Entity::NULL_ENTITY)
                return;

            v8::Local<v8::ObjectTemplate> entityTemp =
            MakeEntityTemplate(args.GetIsolate());

            v8::Local<v8::Object> entityInst = 
            entityTemp->NewInstance(args.GetIsolate()->GetCurrentContext())
            .ToLocalChecked();

            void* handle = reinterpret_cast<void*>(childEntity.GetHandle());
            v8::Local<v8::External> handlePtr =
            v8::External::New(args.GetIsolate(), handle);
            entityInst->SetInternalField(0, handlePtr);

            args.GetReturnValue().Set(entityInst);
            return;

            /**
             * entity.getEntityByName("name")
            */
        }

        void AddChild(const v8::FunctionCallbackInfo<v8::Value>& args)
        {
            if (args.Length() < 1)
            return;

            v8::HandleScope handleScope(args.GetIsolate());

            v8::MaybeLocal<v8::Object> childMaybe =
                args[0]->ToObject(args.GetIsolate()->GetCurrentContext());
            v8::Local<v8::Object> childObject;
            if (!childMaybe.ToLocal(&childObject))
            {
                args.GetReturnValue().Set(false);
                return;
            }

            Entity childEntity = GetExternalEntity(childObject);
            Entity entity = GetExternalEntity(args.Holder());

            entity.GetComponent<EntityHierarchyComponent>().AddChild(childEntity);

            args.GetReturnValue().Set(true);
            return;

            /**
             * true == comp.addChild(entity);
            */
        }

        void GetParent(const v8::FunctionCallbackInfo<v8::Value>& args)
        {
            v8::HandleScope handleScope(args.GetIsolate());

            Entity entity = GetExternalEntity(args.Holder());

            EntityHierarchyComponent& comp =
                entity.GetComponent<EntityHierarchyComponent>();
            
            Entity parent = comp.GetParent();

            if (parent == Entity::NULL_ENTITY)
                return;

            v8::Local<v8::ObjectTemplate> entityTemp =
            MakeEntityTemplate(args.GetIsolate());

            v8::Local<v8::Object> entityInst = 
            entityTemp->NewInstance(args.GetIsolate()->GetCurrentContext())
            .ToLocalChecked();

            void* handle = reinterpret_cast<void*>(parent.GetHandle());
            v8::Local<v8::External> handlePtr =
            v8::External::New(args.GetIsolate(), handle);
            entityInst->SetInternalField(0, handlePtr);

            args.GetReturnValue().Set(entityInst);
            return;

            /**
             * entity = comp.getParent();
             * if (entity == null) {}
            */
        }

        void GetChildren(v8::Local<v8::String> property,
            const v8::PropertyCallbackInfo<v8::Value>& info)
        {
            Entity entity = GetExternalEntity(info.Holder());
            EntityHierarchyComponent& comp =
                entity.GetComponent<EntityHierarchyComponent>();
            std::vector<Entity>& children = comp.children;

            v8::Local<v8::ObjectTemplate> entityTemp =
            MakeEntityTemplate(info.GetIsolate());

            v8::Local<v8::Array> list =
                v8::Array::New(info.GetIsolate(), children.size());

            for (int i = 0; i < children.size(); i++)
            {
                v8::Local<v8::Object> entityInst = 
                entityTemp->NewInstance(info.GetIsolate()->GetCurrentContext())
                .ToLocalChecked();

                void* handle = reinterpret_cast<void*>(children[i].GetHandle());
                v8::Local<v8::External> handlePtr =
                v8::External::New(info.GetIsolate(), handle);
                entityInst->SetInternalField(0, handlePtr);

                list->Set(info.GetIsolate()->GetCurrentContext(),i, entityInst)
                    .ToChecked();
            }
                
            info.GetReturnValue().Set(list);
            return;

            /**
             * list = comp.children
             * list.length
             * Array.isArray(list) == true
             * list[0].hasComponent("Identifier") == true
            */
        }
    } // namespace Hierarchy

    namespace Camera
    {

    } // namespace Camera

    namespace Mesh
    {
        
    } // namespace Mesh

    namespace Light
    {
        
    } // namespace Light
    
}

v8::Local<v8::ObjectTemplate> MakeIdentifierTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);

    v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);

    temp->SetAccessor(v8::String::NewFromUtf8Literal(isolate, "name"),
        JSCallbacks::Identifier::GetName,
        JSCallbacks::Identifier::SetName);
    
    temp->SetInternalFieldCount(1);
    return handleScope.Escape(temp);
}

v8::Local<v8::ObjectTemplate> MakeTransformTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);

    v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);
    //TODO: math lib    
    temp->SetInternalFieldCount(1);
    return handleScope.Escape(temp);
}

v8::Local<v8::ObjectTemplate> MakeHierarchyTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);

    v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);

    temp->Set(isolate, "getParent", v8::FunctionTemplate::New(isolate,
        JSCallbacks::Hierarchy::GetParent));
    temp->Set(isolate, "getEntityByName", v8::FunctionTemplate::New(isolate,
        JSCallbacks::Hierarchy::GetEntityByName));
    temp->Set(isolate, "addChlid", v8::FunctionTemplate::New(isolate,
        JSCallbacks::Hierarchy::AddChild));

    temp->SetAccessor(v8::String::NewFromUtf8Literal(isolate, "children"),
        JSCallbacks::Hierarchy::GetChildren);
    
    temp->SetInternalFieldCount(1);
    return handleScope.Escape(temp);    
}

v8::Local<v8::ObjectTemplate> MakeJavascriptTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);

    v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);
    //TODO:
    temp->SetInternalFieldCount(1);
    return handleScope.Escape(temp);    
}

v8::Local<v8::ObjectTemplate> MakeMeshTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);

    v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);

    // temp->Set(isolate, "loadMesh", v8::FunctionTemplate::New(isolate,
    //     JSCallbacks::Hierarchy::GetParent));
    // temp->Set(isolate, "loadTexture", v8::FunctionTemplate::New(isolate,
    //     JSCallbacks::Hierarchy::GetEntityByName));

    // temp->SetAccessor(v8::String::NewFromUtf8Literal(isolate, "meshPath"),
    //     JSCallbacks::Hierarchy::GetChildren);
    // temp->SetAccessor(v8::String::NewFromUtf8Literal(isolate, "texturePath"),
    //     JSCallbacks::Hierarchy::GetChildren);

    temp->SetInternalFieldCount(1);
    return handleScope.Escape(temp);    
}

v8::Local<v8::ObjectTemplate> MakeCameraTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);

    v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);
    // TODO: math lib
    temp->SetInternalFieldCount(1);
    return handleScope.Escape(temp);    
}

v8::Local<v8::ObjectTemplate> MakeLightTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);

    v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);

    //TODO: math lib
    temp->SetInternalFieldCount(1);
    return handleScope.Escape(temp);    
}