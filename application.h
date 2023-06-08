#pragma once

#include "input.h"
#include "glfw_window.h"
#include "openxr_platform.h"
#include "openxr_swapchain.h"
#include "vulkan_renderer.h"
#include "renderer.h"

#include "objloader.h"

#include "component.h"
#include "light_component.h"
#include "camera_component.h"
#include "mesh_component.h"
#include "ui_component.h"
#include "wireframe_component.h"

#include <Tracy/Tracy.hpp>


class Application
{
public:
    Application();
    ~Application();

    void Run();
    Scene* GetScene();
    
protected:
    virtual void OnCreated() = 0;
    virtual void OnUpdated(float ts) = 0;
    virtual void OnDestroy() = 0;

protected:
    renderer::VulkanRenderer* renderer = nullptr;

private:
    void PollEvents();
    std::vector<const char*> MergeExtensions(
        std::vector<const char*> a, std::vector<const char*> b);

private:
    GlfwWindow* window = nullptr;
    OpenxrPlatform* openxr = nullptr;
    Input* input = nullptr;
    Scene* scene = nullptr;
};