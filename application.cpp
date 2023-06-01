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
        "Current path: " + std::filesystem::current_path().string(),
        Logger::Level::Info, Logger::MsgType::Platform
    );

    GlfwWindow& window = GlfwWindow::GetInstance();
    renderer::VulkanRenderer& renderer = renderer::VulkanRenderer::GetInstance();
    this->openxr = OpenxrPlatform::Initialize();

    window.InitializeWindow();
    renderer.InitializeDevice(
        MergeExtensions(window.GetVkInstanceExt(), openxr->GetVkInstanceExt()), 
        MergeExtensions(window.GetVkDeviceExt(), openxr->GetVkDeviceExt()));
    window.InitializeSurface();

    renderer.AllocateResources(window.GetSwapchain(), openxr->GetSwapchain());
    window.RegisterPeripherals();

    this->renderer = &renderer;
    this->window = &window;

    openxr->InitializeSession(&(renderer.vulkanDevice));
}

Application::~Application()
{
    renderer->DeallocateResources();
    window->DestroySurface();

    renderer->Destroy();
    window->DestroyWindow();

    delete openxr;

    renderer = nullptr;
    window = nullptr;
    openxr = nullptr;
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

std::vector<const char*> Application::MergeExtensions(
    std::vector<const char*> a, std::vector<const char*> b)
{
    for (const char* extension: a)
    {
        bool inList = false;
        for (const char* addedExtension: b)
        {
            if (strcmp(addedExtension, extension) == 0)
            {
                inList = true;
                break;
            }
        }

        if (!inList)
        {
            b.push_back(extension);
        }
    }

    return b;
}