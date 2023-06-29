#pragma once

#include "component.h"

class SceneContext
{
public:
    virtual Component::Type GetSceneContextType() = 0;
    virtual ~SceneContext() {}
};