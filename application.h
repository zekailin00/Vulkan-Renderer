#pragma once

#include "glfw_window.h"
#include "vulkan_renderer.h"

class Application
{
public:
    Application();
    ~Application();

    void Run();
protected:
    virtual void OnCreated() = 0;
    virtual void OnUpdated() = 0;
    virtual void OnDestroy() = 0;

private:
    GlfwWindow* window = nullptr;
    renderer::VulkanRenderer* renderer = nullptr;
};