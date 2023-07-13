#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class SceneContext
{
public:
    
    enum Type
    {
        RendererCtx,
        ScriptCtx,
        CtxSize
    };

public:
    virtual SceneContext::Type GetSceneContextType() = 0;
    virtual ~SceneContext() {}
};

class SceneScriptContext: public SceneContext
{
public:
    SceneContext::Type GetSceneContextType() override {return Type::ScriptCtx;}
    virtual ~SceneScriptContext() {}
};

class SceneRendererContext: public SceneContext
{
public:
    SceneContext::Type GetSceneContextType() override {return Type::RendererCtx;}
    virtual ~SceneRendererContext() {}

public:
    virtual void RenderDebugLine(glm::vec3 direction, float length) = 0;

    virtual void RenderDebugSphere(glm::vec3 position, float radius) = 0;

    virtual void RenderDebugCircle(
        glm::vec3 position, glm::vec3 normal, float radius) = 0;

    virtual void RenderDebugAABB(
        glm::vec3 minCoordinates, glm::vec3 maxCoordinates) = 0;

    virtual void RenderDebugOBB(glm::mat4 transform) = 0;

    virtual void ClearRenderData() = 0;

    virtual void SubmitRenderData() = 0;
};