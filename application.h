#pragma once

#include "input.h"
#include "glfw_window.h"
#include "openxr_platform.h"
#include "openxr_swapchain.h"
#include "vulkan_renderer.h"
#include "renderer.h"

#include "objloader.h"
#include "asset_manager.h"
#include "configuration.h"

#include "component.h"
#include "light_component.h"
#include "camera_component.h"
#include "mesh_component.h"
#include "ui_component.h"
#include "wireframe_component.h"

#include <tracy/Tracy.hpp>
#include <map>


enum SceneHandle
{
    SCENE_XR_SYSTEM_NOT_FOUNT = -1,
    SCENE_VALIDATION_FAILED = -2
};


class Application
{
public:
    Application();
    ~Application();

    void Run();
    Scene* GetActiveScene(int handle);
    int SetActiveScene(Scene* scene);
    Scene* EraseActiveScene(int handle);
    
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
    EventQueue* eventQueue = nullptr;
    GlfwWindow* window = nullptr;
    XrPlatform* openxr = nullptr;
    Input* input = nullptr;

    std::map<int, Scene*> activeScenes;
    int activeSceneHandleCount = 0;
};