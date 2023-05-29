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
    Logger::Write(
        "Current path is " + std::filesystem::current_path().string(),
        Logger::Level::Info,
        Logger::MsgType::Platform
    );

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
    
    Timer timer{};
    while (!window->ShouldClose()) 
    {
        Timestep ts = timer.GetTimestep();

        window->BeginFrame(); // Must before renderer.BeginFrame();
        renderer->BeginFrame();

        {
            ZoneScopedN("Application::OnUpdated");
            OnUpdated(ts);
        }

        renderer->EndFrame();

        FrameMark;
    }
    OnDestroy();
}