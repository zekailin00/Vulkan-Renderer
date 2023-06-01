#include "glfw_window.h"

#include "vulkan_renderer.h"
#include "validation.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <tracy/Tracy.hpp>


void GlfwWindow::InitializeWindow()
{
    ZoneScopedN("GlfwWindow::InitializeWindow");

    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit() || !glfwVulkanSupported())
    {
        Logger::Write(
            "[GLFW Window] Initialization failed.",
            Logger::Level::Error,
            Logger::MsgType::Platform
        );
    }

    uint32_t extensionsCount;
    const char** extensions;
    extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
    vkInstanceExt.resize(extensionsCount);
    memcpy(vkInstanceExt.data(), extensions, extensionsCount * sizeof(const char*));

    Logger::Write(
            "[GLFW Window] Number of extensions needed: " + std::to_string(extensionsCount),
            Logger::Level::Info,
            Logger::MsgType::Platform
        );
    
    for (const char* extension: vkInstanceExt)
    {
        Logger::Write(
            "[GLFW Window] vkInstance extension: " + std::string(extension),
            Logger::Level::Info, Logger::MsgType::Platform
        );
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(1280, 720, "Vulkan Renderer", NULL, NULL);
}

void GlfwWindow::InitializeSurface()
{
    ZoneScopedN("GlfwWindow::InitializeSurface");

    renderer::VulkanRenderer& vkr = renderer::VulkanRenderer::GetInstance();
    CHECK_VKCMD(glfwCreateWindowSurface(vkr.vkInstance, window, nullptr, &surface));

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, glfwFramebufferResizeCallback);

    VkBool32 result;
    vkGetPhysicalDeviceSurfaceSupportKHR(vkr.vulkanDevice.vkPhysicalDevice, vkr.vulkanDevice.graphicsIndex, surface, &result);
    if (result != VK_TRUE)
    {
        Logger::Write(
            "[GLFW Window] Error: no WSI support on physical device.",
            Logger::Level::Error,
            Logger::MsgType::Platform
        );
    }

    windowSwapchain = new WindowSwapchain();
    windowSwapchain->SetSurface(surface);
    windowSwapchain->GetSwapChainProperties();
    windowSwapchain->Initialize();
}

void GlfwWindow::RegisterPeripherals()
{
    // Nothing
}

bool GlfwWindow::ShouldClose()
{
    ZoneScopedN("GlfwWindow::ShouldClose");
    return glfwWindowShouldClose(window);
}

void GlfwWindow::BeginFrame()
{
    ZoneScopedN("GlfwWindow::BeginFrame");
    
    glfwPollEvents();

    // Rebuild swapchain when window size changes
    if (windowSwapchain->swapchainRebuild)
    {
        // Stall the program if window is minimized.
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) 
        {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        renderer::VulkanRenderer::GetInstance().RebuildSwapchain();
        windowSwapchain->swapchainRebuild = false;
    }  
}

void GlfwWindow::DestroySurface()
{
    ZoneScopedN("GlfwWindow::DestroySurface");

    windowSwapchain->Destroy();
    delete windowSwapchain;

    // Destory surface before vkInstance
    renderer::VulkanRenderer& vkr = renderer::VulkanRenderer::GetInstance();
    vkDestroySurfaceKHR(vkr.vkInstance, surface, nullptr);
}

void GlfwWindow::DestroyWindow()
{
    ZoneScopedN("GlfwWindow::DestroyWindow");

    glfwDestroyWindow(window);
    glfwTerminate();
}