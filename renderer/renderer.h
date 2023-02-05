#pragma once 

#include "scene.h"

#include <vector>
#include <memory>

namespace renderer
{

class Renderer
{
public:
    Scene* CreateScene();
    Scene* GetScene();
    
private:
    std::unique_ptr<Scene> scene; // Owner
};

} // namespace renderer