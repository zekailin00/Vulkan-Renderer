#pragma once


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
};