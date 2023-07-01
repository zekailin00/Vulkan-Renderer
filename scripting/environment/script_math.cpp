#include "script_math.h"

#include <v8-function.h>
#include <v8-context.h>
#include <v8-isolate.h>

#include <glm/gtx/euler_angles.hpp>

#include "logger.h"
#include "validation.h"


namespace scripting
{

v8::Local<v8::Object> toV8(
    const glm::vec2& vec2, v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);

    v8::Local<v8::String> prototypeName =
        v8::String::NewFromUtf8(isolate, "Vec2").ToLocalChecked();
    v8::Local<v8::Value> value;
    v8::Local<v8::Function> vec2Function;

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    if (!context->Global()
        ->Get(context, prototypeName).ToLocal(&value) ||
        !value->IsObject())
    {
        Logger::Write(
            "[Scripting] Call failed. Vec2 is Modified.",
            Logger::Level::Warning, Logger::MsgType::Scripting);
        
        return handleScope.Escape(v8::Local<v8::Object>());
    }
    else
    {
        vec2Function = value.As<v8::Function>();
    }

    v8::Local<v8::Value> vec2List[2] = {
        v8::Number::New(isolate, vec2[0]),
        v8::Number::New(isolate, vec2[1]),
    };

    v8::Local<v8::Object> v8Vec2 = vec2Function->NewInstance(
        context, 2, vec2List).ToLocalChecked();

    return handleScope.Escape(v8Vec2);
}

v8::Local<v8::Object> toV8(
    const glm::vec3& vec3, v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);

    v8::Local<v8::String> prototypeName =
        v8::String::NewFromUtf8(isolate, "Vec3").ToLocalChecked();
    v8::Local<v8::Value> value;
    v8::Local<v8::Function> vec3Function;

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    if (!context->Global()
        ->Get(context, prototypeName).ToLocal(&value) ||
        !value->IsObject())
    {
        Logger::Write(
            "[Scripting] Call failed. Vec3 is Modified.",
            Logger::Level::Warning, Logger::MsgType::Scripting);
        
        return handleScope.Escape(v8::Local<v8::Object>());
    }
    else
    {
        vec3Function = value.As<v8::Function>();
    }

    v8::Local<v8::Value> vec3List[3] = {
        v8::Number::New(isolate, vec3.x),
        v8::Number::New(isolate, vec3.y),
        v8::Number::New(isolate, vec3.z)
    };

    v8::Local<v8::Object> v8Vec3 = vec3Function->NewInstance(
        context, 3, vec3List).ToLocalChecked();

    return handleScope.Escape(v8Vec3);
}

v8::Local<v8::Object> toV8(
    const glm::vec4& vec4, v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);

    v8::Local<v8::String> prototypeName =
        v8::String::NewFromUtf8(isolate, "Vec4").ToLocalChecked();
    v8::Local<v8::Value> value;
    v8::Local<v8::Function> vec4Function;

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    if (!context->Global()
        ->Get(context, prototypeName).ToLocal(&value) ||
        !value->IsObject())
    {
        Logger::Write(
            "[Scripting] Call failed. Vec3 is Modified.",
            Logger::Level::Warning, Logger::MsgType::Scripting);
        
        return handleScope.Escape(v8::Local<v8::Object>());
    }
    else
    {
        vec4Function = value.As<v8::Function>();
    }

    v8::Local<v8::Value> vec4List[4] = {
        v8::Number::New(isolate, vec4[0]),
        v8::Number::New(isolate, vec4[1]),
        v8::Number::New(isolate, vec4[2]),
        v8::Number::New(isolate, vec4[3])
    };

    v8::Local<v8::Object> v8Vec4 = vec4Function->NewInstance(
        context, 4, vec4List).ToLocalChecked();

    return handleScope.Escape(v8Vec4);
}

v8::Local<v8::Object> toV8(
    const glm::mat4& mat4, v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);

    v8::Local<v8::String> prototypeName =
        v8::String::NewFromUtf8(isolate, "Mat4").ToLocalChecked();
    v8::Local<v8::Value> value;
    v8::Local<v8::Function> mat4Function;

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    if (!context->Global()
        ->Get(context, prototypeName).ToLocal(&value) ||
        !value->IsObject())
    {
        Logger::Write(
            "[Scripting] Call failed. Mat4 is Modified.",
            Logger::Level::Warning, Logger::MsgType::Scripting);

        return handleScope.Escape(v8::Local<v8::Object>());
    }
    else
    {
        mat4Function = value.As<v8::Function>();
    }

    v8::Local<v8::Value> mat4List[4] = {
        toV8(glm::vec4(mat4[0]), isolate),
        toV8(glm::vec4(mat4[1]), isolate),
        toV8(glm::vec4(mat4[2]), isolate),
        toV8(glm::vec4(mat4[3]), isolate),
    };

    v8::Local<v8::Object> v8Mat4 = mat4Function->NewInstance(
        context, 4, mat4List).ToLocalChecked();
    
    return handleScope.Escape(v8Mat4);
}

bool toCpp(glm::vec2& vec2, v8::Local<v8::Object> v8Vec2,
    v8::Isolate* isolate)
{
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::String> v8X =
        v8::String::NewFromUtf8(isolate, "x")
        .ToLocalChecked();

    v8::Local<v8::String> v8Y =
        v8::String::NewFromUtf8(isolate, "y")
        .ToLocalChecked();

    v8::Local<v8::Value> v8XValue;
    v8::Local<v8::Value> v8YValue;

    if (v8Vec2->Get(context, v8X).ToLocal(&v8XValue) &&
        v8Vec2->Get(context, v8Y).ToLocal(&v8YValue) &&
        v8XValue->IsNumber() && v8YValue->IsNumber())
    {
        vec2[0] = v8XValue->ToNumber(context).ToLocalChecked()->Value();
        vec2[1] = v8YValue->ToNumber(context).ToLocalChecked()->Value();

        return true;
    }
    else
    {
        return false;
    }
}

bool toCpp(glm::vec3& vec3, v8::Local<v8::Object> v8Vec3,
    v8::Isolate* isolate)
{
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::String> v8X =
        v8::String::NewFromUtf8(isolate, "x")
        .ToLocalChecked();

    v8::Local<v8::String> v8Y =
        v8::String::NewFromUtf8(isolate, "y")
        .ToLocalChecked();

    v8::Local<v8::String> v8Z =
        v8::String::NewFromUtf8(isolate, "z")
        .ToLocalChecked();

    v8::Local<v8::Value> v8XValue;
    v8::Local<v8::Value> v8YValue;
    v8::Local<v8::Value> v8ZValue;

    if (v8Vec3->Get(context, v8X).ToLocal(&v8XValue) &&
        v8Vec3->Get(context, v8Y).ToLocal(&v8YValue) &&
        v8Vec3->Get(context, v8Z).ToLocal(&v8ZValue) &&
        v8XValue->IsNumber() && v8YValue->IsNumber() && v8ZValue->IsNumber())
    {
        vec3[0] = v8XValue->ToNumber(context).ToLocalChecked()->Value();
        vec3[1] = v8YValue->ToNumber(context).ToLocalChecked()->Value();
        vec3[2] = v8ZValue->ToNumber(context).ToLocalChecked()->Value();

        return true;
    }
    else
    {
        return false;
    }
}

bool toCpp(glm::vec4& vec4, v8::Local<v8::Object> v8Vec4,
    v8::Isolate* isolate)
{
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::String> v8X =
        v8::String::NewFromUtf8(isolate, "x")
        .ToLocalChecked();

    v8::Local<v8::String> v8Y =
        v8::String::NewFromUtf8(isolate, "y")
        .ToLocalChecked();

    v8::Local<v8::String> v8Z =
        v8::String::NewFromUtf8(isolate, "z")
        .ToLocalChecked();
    
    v8::Local<v8::String> v8W =
        v8::String::NewFromUtf8(isolate, "w")
        .ToLocalChecked();

    v8::Local<v8::Value> v8XValue;
    v8::Local<v8::Value> v8YValue;
    v8::Local<v8::Value> v8ZValue;
    v8::Local<v8::Value> v8WValue;

    if (v8Vec4->Get(context, v8X).ToLocal(&v8XValue) &&
        v8Vec4->Get(context, v8Y).ToLocal(&v8YValue) &&
        v8Vec4->Get(context, v8Z).ToLocal(&v8ZValue) &&
        v8Vec4->Get(context, v8W).ToLocal(&v8WValue) &&
        v8XValue->IsNumber() && v8YValue->IsNumber() &&
        v8ZValue->IsNumber() && v8WValue->IsNumber())
    {
        vec4[0] = v8XValue->ToNumber(context).ToLocalChecked()->Value();
        vec4[1] = v8YValue->ToNumber(context).ToLocalChecked()->Value();
        vec4[2] = v8ZValue->ToNumber(context).ToLocalChecked()->Value();
        vec4[3] = v8WValue->ToNumber(context).ToLocalChecked()->Value();

        return true;
    }
    else
    {
        return false;
    }
}

bool toCpp(glm::mat4& mat4, v8::Local<v8::Object> v8Mat4,
    v8::Isolate* isolate)
{
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::String> v8C0 =
        v8::String::NewFromUtf8(isolate, "c0")
        .ToLocalChecked();

    v8::Local<v8::String> v8C1 =
        v8::String::NewFromUtf8(isolate, "c1")
        .ToLocalChecked();

    v8::Local<v8::String> v8C2 =
        v8::String::NewFromUtf8(isolate, "c2")
        .ToLocalChecked();
    
    v8::Local<v8::String> v8C3 =
        v8::String::NewFromUtf8(isolate, "c3")
        .ToLocalChecked();

    v8::Local<v8::Value> v8C0Value;
    v8::Local<v8::Value> v8C1Value;
    v8::Local<v8::Value> v8C2Value;
    v8::Local<v8::Value> v8C3Value;

    if (v8Mat4->Get(context, v8C0).ToLocal(&v8C0Value) &&
        v8Mat4->Get(context, v8C1).ToLocal(&v8C1Value) &&
        v8Mat4->Get(context, v8C2).ToLocal(&v8C2Value) &&
        v8Mat4->Get(context, v8C3).ToLocal(&v8C3Value) &&
        v8C0Value->IsObject() && v8C1Value->IsObject() &&
        v8C2Value->IsObject() && v8C3Value->IsObject())
    {
        bool result = true;
        glm::mat4 matrix;

        result = result && toCpp(matrix[0],
            v8C0Value->ToObject(context).ToLocalChecked(), isolate);
        result = result && toCpp(matrix[1],
            v8C1Value->ToObject(context).ToLocalChecked(), isolate);
        result = result && toCpp(matrix[2],
            v8C2Value->ToObject(context).ToLocalChecked(), isolate);
        result = result && toCpp(matrix[3],
            v8C3Value->ToObject(context).ToLocalChecked(), isolate);

        if (result)
        {
            mat4 = matrix;
        }

        return result;
    }
    else
    {
        return false;
    }
}

namespace math
{

bool GetBinaryMat4Operants(
    const v8::FunctionCallbackInfo<v8::Value> &info,
    glm::mat4& lhs, glm::mat4& rhs
)
{
    if (info.Length() != 2 ||
        !info[0]->IsObject() || !info[1]->IsObject())
    {
        Logger::Write(
            "[Scripting] Mat4 operants are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        
        return false;
    }

    bool result = true;
    result = result && toCpp(lhs, info[0].As<v8::Object>(), info.GetIsolate());
    result = result && toCpp(rhs, info[1].As<v8::Object>(), info.GetIsolate());

    if (!result)
    {
        Logger::Write(
            "[Scripting] Mat4 operants are invalid",
            Logger::Level::Warning, Logger::Scripting
        );

        return false;
    }

    return true;
}

void Mat4Add(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    glm::mat4 lhs, rhs, retval;
    v8::Local<v8::Object> v8Retval;
    
    if (!GetBinaryMat4Operants(info, lhs, rhs))
    {
        return;
    }

    retval = lhs + rhs;
    v8Retval = toV8(retval, info.GetIsolate());

    ASSERT(!v8Retval.IsEmpty());
    info.GetReturnValue().Set(v8Retval);
}

void Mat4Multiply(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    glm::mat4 lhs, rhs, retval;
    v8::Local<v8::Object> v8Retval;
    
    if (!GetBinaryMat4Operants(info, lhs, rhs))
    {
        return;
    }

    retval = lhs * rhs;
    v8Retval = toV8(retval, info.GetIsolate());

    ASSERT(!v8Retval.IsEmpty());
    info.GetReturnValue().Set(v8Retval);
}

void Mat4Subtract(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    glm::mat4 lhs, rhs, retval;
    v8::Local<v8::Object> v8Retval;
    
    if (!GetBinaryMat4Operants(info, lhs, rhs))
    {
        return;
    }

    retval = lhs - rhs;
    v8Retval = toV8(retval, info.GetIsolate());

    ASSERT(!v8Retval.IsEmpty());
    info.GetReturnValue().Set(v8Retval);
}

void Mat4Inverse(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    if (info.Length() != 1 || !info[0]->IsObject())
    {
        Logger::Write(
            "[Scripting] Mat4Inverse parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    glm::mat4 operant;
    bool result = toCpp(operant, info[0].As<v8::Object>(), info.GetIsolate());

    if (!result)
    {
        Logger::Write(
            "[Scripting] Mat4Inverse parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );

        return;
    }

    operant = glm::inverse(operant);
    v8::Local<v8::Object> v8Retval = toV8(operant, info.GetIsolate());

    ASSERT(!v8Retval.IsEmpty());
    info.GetReturnValue().Set(v8Retval);
}

void Mat4Identity(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    glm::mat4 retval(1.0);
    v8::Local<v8::Object> v8Retval = toV8(retval, info.GetIsolate());

    ASSERT(!v8Retval.IsEmpty());
    info.GetReturnValue().Set(v8Retval);
}

/* EulerAngleXYZ */
void Mat4Rotation(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    if (info.Length() != 1 || !info[0]->IsObject())
    {
        Logger::Write(
            "[Scripting] Mat4Rotation parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    glm::vec3 rotation;
    bool result = toCpp(rotation, info[0].As<v8::Object>(), info.GetIsolate());

    if (!result)
    {
        Logger::Write(
            "[Scripting] Mat4Rotation parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );

        return;
    }

    glm::mat4 rotationMat =
        glm::eulerAngleXYZ(rotation[0], rotation[1], rotation[2]);
    
    v8::Local<v8::Object> v8Retval = toV8(rotationMat, info.GetIsolate());
    ASSERT(!v8Retval.IsEmpty());
    info.GetReturnValue().Set(v8Retval);
}

void Mat4Translate(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    if (info.Length() != 1 || !info[0]->IsObject())
    {
        Logger::Write(
            "[Scripting] Mat4Translate parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    glm::vec3 translate;
    bool result = toCpp(translate, info[0].As<v8::Object>(), info.GetIsolate());

    if (!result)
    {
        Logger::Write(
            "[Scripting] Mat4Translate parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );

        return;
    }

    glm::mat4 translateMat = glm::translate(glm::mat4(1.0f), translate);
    
    v8::Local<v8::Object> v8Retval = toV8(translateMat, info.GetIsolate());
    ASSERT(!v8Retval.IsEmpty());
    info.GetReturnValue().Set(v8Retval);
}

void Mat4Scale(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    if (info.Length() != 1 || !info[0]->IsObject())
    {
        Logger::Write(
            "[Scripting] Mat4Scale parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    glm::vec3 scale;
    bool result = toCpp(scale, info[0].As<v8::Object>(), info.GetIsolate());

    if (!result)
    {
        Logger::Write(
            "[Scripting] Mat4Scale parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );

        return;
    }

    glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);
    
    v8::Local<v8::Object> v8Retval = toV8(scaleMat, info.GetIsolate());
    ASSERT(!v8Retval.IsEmpty());
    info.GetReturnValue().Set(v8Retval);
}


} // namespace math

} // namespace scripting