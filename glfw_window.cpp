#include "glfw_window.h"

#include "imgui_impl_glfw.h"
#include "vulkan_renderer.h"
#include "validation.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>


GlfwWindow::GlfwWindow()
{
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit() || !glfwVulkanSupported())
    {
        std::cout << "[GLFW Window] Initialization failed.\n";
        exit(1);
    }
    extensionsCount = 0;
    extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);

    std::cout << "[GLFW Window] Number of extensions needed: " << extensionsCount << std::endl;
}

void GlfwWindow::InitializeWindow()
{
    VulkanRenderer& vkr = VulkanRenderer::GetInstance();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(1280, 720, "Vulkan Renderer", NULL, NULL);
    CHECK_VKCMD(glfwCreateWindowSurface(vkr.vkInstance, window, nullptr, &surface));

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, glfwFramebufferResizeCallback);

    VkBool32 result;
    vkGetPhysicalDeviceSurfaceSupportKHR(vkr.vulkanDevice.vkPhysicalDevice, vkr.vulkanDevice.graphicsIndex, surface, &result);
    if (result != VK_TRUE)
    {
        std::cout << "[GLFW Window] Error: no WSI support on physical device." << std::endl;
        exit(1);
    }

    windowSwapchain.SetSurface(surface);
    windowSwapchain.GetSwapChainProperties();
}

void GlfwWindow::RegisterPeripherals()
{
    ImGui_ImplGlfw_InitForVulkan(window, true);
}

bool GlfwWindow::ShouldClose()
{
    return glfwWindowShouldClose(window);
}

void GlfwWindow::BeginFrame()
{
    glfwPollEvents();

    // Rebuild swapchain when window size changes
    if (windowSwapchain.swapchainRebuild)
    {
        // Stall the program if window is minimized.
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) 
        {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        VulkanRenderer::GetInstance().RebuildSwapchain();
        windowSwapchain.swapchainRebuild = false;
    }

    ImGui_ImplGlfw_NewFrame();     
}

void GlfwWindow::CloseWindow()
{
    ImGui_ImplGlfw_Shutdown();
}

void GlfwWindow::Destroy()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}