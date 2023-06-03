#include <iostream>
#include <filesystem>

#include "application.h"
#include "logger.h"
#include "timestep.h"

bool launchXR = true;

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
    this->input = Input::GetInstance();
    this->openxr = OpenxrPlatform::Initialize(this->input);

    window.InitializeWindow();
    renderer.InitializeDevice(
        MergeExtensions(window.GetVkInstanceExt(), openxr->GetVkInstanceExt()), 
        MergeExtensions(window.GetVkDeviceExt(), openxr->GetVkDeviceExt()));

    window.InitializeSurface();
    renderer.AllocateResources(window.GetSwapchain(), openxr->GetSwapchain());

    this->renderer = &renderer;
    this->window = &window;

}

Application::~Application()
{
    if (launchXR)
    {
        renderer->DestroyXrSession();
    }

    renderer->DeallocateResources();
    window->DestroySurface();

    renderer->Destroy();
    window->DestroyWindow();

    delete openxr; //TODO: Destroy resources

    renderer = nullptr;
    window = nullptr;
    openxr = nullptr;
}

void Application::PollEvents()
{
    //TODO:
    openxr->PollEvents();
    openxr->ShouldCloseSeesion();
}

void Application::Run()
{
    OnCreated();

    if (launchXR)
    {
        renderer->InitializeXrSession(openxr->NewSession());
        // TODO: intialization failed
    }
    
    Timer timer{};
    while (!window->ShouldClose()) 
    {
        Timestep ts = timer.GetTimestep();
        PollEvents();

        window->BeginFrame(); // Must before renderer.BeginFrame();
        renderer->BeginFrame();
        openxr->BeginFrame();
        

        {
            ZoneScopedN("Application::OnUpdated");
            OnUpdated(ts);
        }

        renderer->EndFrame();
        openxr->EndFrame();

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