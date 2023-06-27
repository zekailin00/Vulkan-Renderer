#include "environment.h"

#include "entity.h"
#include "experience.h"
#include "core_components.h"
#include "renderer_components.h"

#include "component_templates.h"

#include <v8-external.h>
#include <v8-function-callback.h>
#include <iostream> //debug

// component = entity.GetComponent("Transform");
// component.Method();
// component.properties = 333;
// root.GetChildByName()
// system.log()
// scene.methods()
/**
 * Transform
 * Hierarchy
 * Identifier
 * Javascript
 */ 
/**
 * root: Entity
 * - GetComponent(string): component
 * - GetChildByName(string): entity
 */

/**
 * For global objects 
 * 1. create a template
 * 2. instantiate context with global object
 * 3. instance a object from the template
 * 4. bind the object with c++ instance
 * 5. bind the object to the global object
 * 
 * For system template 
 * - bind function template 
 * 
 * For entity template
 * - bind function templates
 * - set external size = 1
 * - bind entity whenever an object is instantiated
 * 
 * For component template
 * - 
*/

Entity GetExternalEntity(const v8::Local<v8::Object>& obj)
{
  v8::Local<v8::External> field = 
    obj->GetInternalField(0).As<v8::External>();

  void* ptr = field->Value();
  EntityHandle* handle = reinterpret_cast<EntityHandle*>(&ptr);
  Experience& exp = Experience::GetInstance();
  Entity entity = exp.GetEntityFromHandle(*handle);

  return entity;
}

namespace JSCallbacks
{
  void log(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    bool first = true;
    for (int i = 0; i < args.Length(); i++) {
      v8::HandleScope handle_scope(args.GetIsolate());
      if (first) {
        first = false;
        std::cout << "[Javascript] ";
      } else {
        std::cout << " ";
      }
      v8::String::Utf8Value str(args.GetIsolate(), args[i]);
      std::cout << *str;
    }
    std::cout << std::endl;
  }

  void getEntityByName(const v8::FunctionCallbackInfo<v8::Value>& args)
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

  void getComponent(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    Entity entity = GetExternalEntity(args.Holder());
    
    if (args.Length() < 1)
      return;
    
    v8::String::Utf8Value nameStr(args.GetIsolate(), args[0]);
    std::string name = *nameStr;
    v8::Local<v8::Object> component;

    if (name == "Identifier")
    {
      if (!entity.HasComponent<IdentifierComponent>())
        return;

      component = MakeIdentifierTemplate(args.GetIsolate())
        ->NewInstance(args.GetIsolate()->GetCurrentContext())
        .ToLocalChecked();
    }
    else if (name == "Transform")
    {
      if (!entity.HasComponent<TransformComponent>())
        return;

      component = MakeTransformTemplate(args.GetIsolate())
        ->NewInstance(args.GetIsolate()->GetCurrentContext())
        .ToLocalChecked();
    }
    else if (name == "Hierarchy")
    {
      if (!entity.HasComponent<EntityHierarchyComponent>())
        return;

      component = MakeHierarchyTemplate(args.GetIsolate())
        ->NewInstance(args.GetIsolate()->GetCurrentContext())
        .ToLocalChecked();
    }
    else if (name == "Javascript")
    {
      if (!entity.HasComponent<JavascriptComponent>())
        return;

      component = MakeJavascriptTemplate(args.GetIsolate())
        ->NewInstance(args.GetIsolate()->GetCurrentContext())
        .ToLocalChecked();
    }
    else if (name == "Mesh")
    {
      if (!entity.HasComponent<MeshComponent>())
        return;

      component = MakeMeshTemplate(args.GetIsolate())
        ->NewInstance(args.GetIsolate()->GetCurrentContext())
        .ToLocalChecked();
    }
    else if (name == "Camera")
    {
      if (!entity.HasComponent<CameraComponent>())
        return;

      component = MakeCameraTemplate(args.GetIsolate())
        ->NewInstance(args.GetIsolate()->GetCurrentContext())
        .ToLocalChecked();
    }
    else if (name == "Light")
    {
      if (!entity.HasComponent<LightComponent>())
        return;

      component = MakeLightTemplate(args.GetIsolate())
        ->NewInstance(args.GetIsolate()->GetCurrentContext())
        .ToLocalChecked();
    }

    // No component is found with given string
    if (component.IsEmpty())
      return;

    v8::Local<v8::External> ptr = v8::External::New(args.GetIsolate(), 
      reinterpret_cast<void*>(entity.GetHandle()));
    component->SetInternalField(0, ptr);

    args.GetReturnValue().Set(component);
    return;
  }

  void hasComponent(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    if (args.Length() < 1)
      return;
    v8::HandleScope handleScope(args.GetIsolate());

    v8::Local<v8::Object> obj = args.Holder();
    v8::Local<v8::External> field = 
      obj->GetInternalField(0).As<v8::External>();

    void* ptr = field->Value();
    EntityHandle* handle = reinterpret_cast<EntityHandle*>(&ptr);
    Experience& exp = Experience::GetInstance();
    Entity entity = exp.GetEntityFromHandle(*handle);

    v8::String::Utf8Value nameStr(args.GetIsolate(), args[0]);
    std::string name = *nameStr;

    if (name == "Identifier")
    {
      bool retval = entity.HasComponent<IdentifierComponent>();
      args.GetReturnValue().Set(retval);
    }
    else if (name == "Transform")
    {
      bool retval = entity.HasComponent<TransformComponent>();
      args.GetReturnValue().Set(retval);
    }
    else if (name == "Hierarchy")
    {
      bool retval = entity.HasComponent<EntityHierarchyComponent>();
      args.GetReturnValue().Set(retval);
    }
    else if (name == "Javascript")
    {
      bool retval = entity.HasComponent<JavascriptComponent>();
      args.GetReturnValue().Set(retval);
    }
    else if (name == "Mesh")
    {
      bool retval = entity.HasComponent<MeshComponent>();
      args.GetReturnValue().Set(retval);
    }
    else if (name == "Camera")
    {
      bool retval = entity.HasComponent<CameraComponent>();
      args.GetReturnValue().Set(retval);
    }
    else if (name == "Light")
    {
      bool retval = entity.HasComponent<LightComponent>();
      args.GetReturnValue().Set(retval);
    }

    return;
  }

  void removeComponent(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    if (args.Length() < 1)
    {
      args.GetReturnValue().Set(false);
      return;
    }

    Entity entity = GetExternalEntity(args.Holder());

    v8::String::Utf8Value nameStr(args.GetIsolate(), args[0]);
    std::string name = *nameStr;

    if (name == "Identifier")
    { // Cannot be removed
      args.GetReturnValue().Set(false);
    }
    else if (name == "Transform")
    { // Cannot be removed
      args.GetReturnValue().Set(false);
    }
    else if (name == "Hierarchy")
    { // Cannot be removed
      args.GetReturnValue().Set(false);
    }
    else if (name == "Javascript")
    {
      if (!entity.HasComponent<JavascriptComponent>())
      {      
        args.GetReturnValue().Set(false);
      }
      else 
      {
        entity.RemoveComponent<JavascriptComponent>();
        args.GetReturnValue().Set(true);
      }
    }
    else if (name == "Mesh")
    {
      if (!entity.HasComponent<MeshComponent>())
      {      
        args.GetReturnValue().Set(false);
      }
      else 
      {
        entity.RemoveComponent<MeshComponent>();
        args.GetReturnValue().Set(true);
      }
    }
    else if (name == "Camera")
    {
      if (!entity.HasComponent<CameraComponent>())
      {      
        args.GetReturnValue().Set(false);
      }
      else 
      {
        entity.RemoveComponent<CameraComponent>();
        args.GetReturnValue().Set(true);
      }
    }
    else if (name == "Light")
    {
      if (!entity.HasComponent<LightComponent>())
      {      
        args.GetReturnValue().Set(false);
      }
      else 
      {
        entity.RemoveComponent<LightComponent>();
        args.GetReturnValue().Set(true);
      }
    }

    return;

    /**
     * entity.removeComponent("Identifier") == false // not removable
     * entity.removeComponent("not-exist") == false
     * 
    */
  }

  void addComponent(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    if (args.Length() < 1)
      return;

    Entity entity = GetExternalEntity(args.Holder());

    v8::String::Utf8Value nameStr(args.GetIsolate(), args[0]);
    std::string name = *nameStr;


    v8::Local<v8::Object> component;
    if (name == "Javascript")
    {
      if (entity.HasComponent<JavascriptComponent>())
        return;
      
      entity.AddComponent<JavascriptComponent>();

      component = MakeJavascriptTemplate(args.GetIsolate())
      ->NewInstance(args.GetIsolate()->GetCurrentContext())
      .ToLocalChecked();

    }
    else if (name == "Mesh")
    {
      if (entity.HasComponent<MeshComponent>())
        return;
      
      entity.AddComponent<MeshComponent>();

      component = MakeMeshTemplate(args.GetIsolate())
      ->NewInstance(args.GetIsolate()->GetCurrentContext())
      .ToLocalChecked();
    }
    else if (name == "Camera")
    {
      if (entity.HasComponent<CameraComponent>())
        return;
      
      entity.AddComponent<CameraComponent>();

      component = MakeCameraTemplate(args.GetIsolate())
      ->NewInstance(args.GetIsolate()->GetCurrentContext())
      .ToLocalChecked();
    }
    else if (name == "Light")
    {
      if (entity.HasComponent<LightComponent>())
        return;
      
      entity.AddComponent<LightComponent>();

      component = MakeLightTemplate(args.GetIsolate())
      ->NewInstance(args.GetIsolate()->GetCurrentContext())
      .ToLocalChecked();
    }

    // No component is found with given string
    if (component.IsEmpty())
      return;

    v8::Local<v8::External> ptr = v8::External::New(args.GetIsolate(), 
      reinterpret_cast<void*>(entity.GetHandle()));
    component->SetInternalField(0, ptr);

    args.GetReturnValue().Set(component);
    return;

    /**
     * entity.addComponent("Identifier") == null; //alreadu added
     * entity.addComponent("Mesh") == meshComp
     * meshComp.meshPath
    */
  }

  void getRootEntity(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    Entity entity = GetExternalEntity(args.Holder());

    EntityHierarchyComponent& comp =
      entity.GetComponent<EntityHierarchyComponent>();

    // There is always a root entity.
    // Never be Entity::NULL_ENTITY
    Entity rootEntity = comp.GetRootEntity();

    v8::Local<v8::ObjectTemplate> entityTemp =
      MakeEntityTemplate(args.GetIsolate());

    v8::Local<v8::Object> entityInst = 
      entityTemp->NewInstance(args.GetIsolate()->GetCurrentContext())
      .ToLocalChecked();

    void* handle = reinterpret_cast<void*>(rootEntity.GetHandle());
    v8::Local<v8::External> handlePtr =
      v8::External::New(args.GetIsolate(), handle);
    entityInst->SetInternalField(0, handlePtr);

    args.GetReturnValue().Set(entityInst);

    /**
     * scene.getRootEntity()
    */
  }

  void newEntity(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    Entity entity = GetExternalEntity(args.Holder());

    Entity newEntity = entity.AddChild();

    v8::Local<v8::ObjectTemplate> entityTemp =
      MakeEntityTemplate(args.GetIsolate());

    v8::Local<v8::Object> entityInst = 
      entityTemp->NewInstance(args.GetIsolate()->GetCurrentContext())
      .ToLocalChecked();

    void* handle = reinterpret_cast<void*>(newEntity.GetHandle());
    v8::Local<v8::External> handlePtr =
      v8::External::New(args.GetIsolate(), handle);
    entityInst->SetInternalField(0, handlePtr);

    args.GetReturnValue().Set(entityInst);
    return;
    
    /**
     * newEntity = entity.newEntity()
     * newEntity.getComponent<Hierarchy>().parent == entity
    */
  }

}

v8::Local<v8::ObjectTemplate> MakeSystemTemplate(v8::Isolate* isolate)
{
  v8::EscapableHandleScope handleScope(isolate);

  v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);
  temp->Set(isolate, "log", 
    v8::FunctionTemplate::New(isolate, JSCallbacks::log));

  return handleScope.Escape(temp);
}

v8::Local<v8::ObjectTemplate> MakeEntityTemplate(v8::Isolate* isolate)
{
  v8::EscapableHandleScope handleScope(isolate);

  v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);
  temp->Set(isolate, "getComponent",
    v8::FunctionTemplate::New(isolate, JSCallbacks::getComponent));
  temp->Set(isolate, "hasComponent",
    v8::FunctionTemplate::New(isolate, JSCallbacks::hasComponent));
  temp->Set(isolate, "addComponent",
    v8::FunctionTemplate::New(isolate, JSCallbacks::addComponent));
  temp->Set(isolate, "removeComponent",
    v8::FunctionTemplate::New(isolate, JSCallbacks::removeComponent));
  temp->Set(isolate, "getEntityByName",
    v8::FunctionTemplate::New(isolate, JSCallbacks::getEntityByName));
  temp->Set(isolate, "getRootEntity",
    v8::FunctionTemplate::New(isolate, JSCallbacks::getRootEntity));
  temp->Set(isolate, "newEntity",
    v8::FunctionTemplate::New(isolate, JSCallbacks::newEntity));

  temp->SetInternalFieldCount(1);
  return handleScope.Escape(temp);
}

v8::Local<v8::ObjectTemplate> MakeSceneTemplate(v8::Isolate* isolate)
{
  v8::EscapableHandleScope handleScope(isolate);

  v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);

  return handleScope.Escape(temp);
}

v8::Local<v8::ObjectTemplate> MakeConsoleTemplate(v8::Isolate* isolate)
{
  v8::EscapableHandleScope handleScope(isolate);

  v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);

  return handleScope.Escape(temp);
}