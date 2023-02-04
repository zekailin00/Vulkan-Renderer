#pragma once

#include "glfw_window.h"
#include "vulkan_renderer.h"

class Application
{
public:
    Application();
    ~Application();

    void Run();
    virtual void Build() = 0;

private:
    GlfwWindow* window = nullptr;
    VulkanRenderer* renderer = nullptr;
};