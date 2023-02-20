#pragma once

#include "glfw_window.h"
#include "vulkan_renderer.h"
#include "renderer.h"


class Application
{
public:
    Application();
    ~Application();

    void Run();
    renderer::Node* GetRootNode();
    
protected:
    virtual void OnCreated() = 0;
    virtual void OnUpdated() = 0;
    virtual void OnDestroy() = 0;

protected:
    renderer::VulkanRenderer* renderer = nullptr;

private:
    GlfwWindow* window = nullptr;
};