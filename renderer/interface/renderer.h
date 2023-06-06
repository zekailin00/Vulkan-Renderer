#pragma once 

#include "scene.h"

#include <vector>
#include <memory>

namespace renderer
{

class Renderer
{
public:
    virtual Scene* CreateScene() = 0;
    virtual Scene* GetScene() = 0;
    
protected:
    std::unique_ptr<Scene> scene; // Owner
};

} // namespace renderer