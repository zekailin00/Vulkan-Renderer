#pragma once 

#define GLFW_INCLUDE_VULKAN
#include "glfw/glfw3.h"
#include "vulkan/vulkan.h"

#include "window_swapchain.h"


class GlfwWindow {
public:
    static GlfwWindow& GetInstance()
    {
        static GlfwWindow glfwWindow;
        return glfwWindow;
    }

    void InitializeWindow();
    void RegisterPeripherals();
    bool ShouldClose();
    void BeginFrame();
    void CloseWindow();
    void Destroy();
    IVulkanSwapchain* GetSwapchain() {return &windowSwapchain;}

    uint32_t extensionsCount;
    const char** extensions;

    GlfwWindow(GlfwWindow const&) = delete;
    void operator=(GlfwWindow const&) = delete;

private:
    GlfwWindow();
    
    static void glfwFramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto glfwWindow = reinterpret_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
        glfwWindow->windowSwapchain.swapchainRebuild = true;
    }

    static void glfwErrorCallback(int error, const char* description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }

    GLFWwindow* window;
    VkSurfaceKHR surface;
    WindowSwapchain windowSwapchain;
};