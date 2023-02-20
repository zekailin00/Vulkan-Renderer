#include <iostream>
#include <filesystem>

#include "application.h"
#include "logger.h"


Application::Application()
{
    std::cout << "Current path is " << std::filesystem::current_path() << '\n';
    GlfwWindow& window = GlfwWindow::GetInstance();
    renderer::VulkanRenderer& renderer = renderer::VulkanRenderer::GetInstance();

    renderer.InitializeDevice(window.extensionsCount, window.extensions);
    window.InitializeWindow();

    renderer.AllocateResources(window.GetSwapchain());
    window.RegisterPeripherals();

    this->renderer = &renderer;
    this->window = &window;
}

Application::~Application()
{
    renderer->DeallocateResources();
    window->CloseWindow();

    renderer->Destroy();
    window->Destroy();

    renderer = nullptr;
    window = nullptr;
}

void Application::Run()
{
    OnCreated();
    while (!window->ShouldClose()) 
    {
        window->BeginFrame(); // Must before renderer.BeginFrame();
        renderer->BeginFrame();
        OnUpdated();
        renderer->EndFrame();
    }
    OnDestroy();
}