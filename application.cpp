#include <iostream>
#include <filesystem>

#include "application.h"
#include "logger.h"
#include "timestep.h"


renderer::Node* Application::GetRootNode()
{
    return renderer->GetScene()->GetRootNode();
}

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
    
    Timer timer{60}; //FIXME: running as fast as possible breaks uniform coherency!!
    while (!window->ShouldClose()) 
    {
        Timestep ts = timer.GetTimestep();

        window->BeginFrame(); // Must before renderer.BeginFrame();
        renderer->BeginFrame();
        OnUpdated(ts);
        renderer->EndFrame();
    }
    OnDestroy();
}