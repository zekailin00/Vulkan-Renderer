#pragma once 

#define GLFW_INCLUDE_VULKAN
#include "glfw/glfw3.h"
#include "vulkan/vulkan.h"

#include "glfw_swapchain.h"

#include <vector>


class GlfwWindow {
public:
    static GlfwWindow& GetInstance()
    {
        static GlfwWindow glfwWindow;
        return glfwWindow;
    }

    void InitializeWindow();
    void InitializeSurface();
    void RegisterPeripherals();
    bool ShouldClose();
    void BeginFrame();
    void DestroySurface();
    void DestroyWindow();

    IVulkanSwapchain* GetSwapchain() {return windowSwapchain;}
    std::vector<const char*> GetVkInstanceExt() {return vkInstanceExt;}
    std::vector<const char*> GetVkDeviceExt() {return {"VK_KHR_swapchain"};}

    GlfwWindow(GlfwWindow const&) = delete;
    void operator=(GlfwWindow const&) = delete;

private:
    GlfwWindow() = default;
    ~GlfwWindow() = default;
    
    static void glfwFramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto glfwWindow = reinterpret_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
        glfwWindow->windowSwapchain->swapchainRebuild = true;
    }

    static void glfwErrorCallback(int error, const char* description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }

    GLFWwindow* window;
    VkSurfaceKHR surface;
    WindowSwapchain* windowSwapchain;
    std::vector<const char*> vkInstanceExt{};
};