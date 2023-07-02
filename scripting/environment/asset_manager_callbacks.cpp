#include "asset_manager_callbacks.h"

#include <v8-external.h>
#include <v8-object.h>
#include <v8-template.h>
#include <v8-function.h>
#include <v8-primitive.h>
#include <v8-container.h>

#include "entity.h"
#include "component.h"

#include "scripting_subsystem.h"

#include "logger.h"
#include "validation.h"

#include "renderer_asset_manager.h"
#include "asset_manager.h"


namespace scripting
{

void GetMeshResourcePaths(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    //??? FIXME: need to redesign asset manager system!!
    renderer::IRendererAssetManager* assetManager =
        dynamic_cast<renderer::IRendererAssetManager*>(
            dynamic_cast<ICoreAssetManager*>(
                static_cast<IScriptAssetManager*>(field->Value())));
    ASSERT(assetManager != nullptr);

    std::vector<const char *> meshPaths{};
    assetManager->GetAvailableMeshes(meshPaths);


    v8::Local<v8::Array> v8MeshPaths = v8::Array::New(isolate, meshPaths.size());
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    int i = 0;
    for (const char * path: meshPaths)
    {
        v8::Local<v8::String> v8Path =
            v8::String::NewFromUtf8(isolate, path).ToLocalChecked();

        v8MeshPaths->Set(context, i, v8Path).ToChecked();
        i++;
    }

    info.GetReturnValue().Set(v8MeshPaths);
}

} // namespace scripting
