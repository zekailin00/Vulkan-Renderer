#include <iostream>
#include <filesystem>

#include "application.h"
#include "logger.h"
#include "timestep.h"
#include "validation.h"

bool launchXR = false;

Scene* Application::GetActiveScene(int handle)
{
    ZoneScopedN("Application::GetActiveScene");

    Scene* scene = activeScenes[handle];
    ASSERT(scene != nullptr);
    return scene;
}

int Application::SetActiveScene(Scene* scene)
{
    ZoneScopedN("Application::SetActiveScene");

    int handle = activeSceneHandleCount++;
    ASSERT(scene != nullptr);
    ASSERT(activeScenes.find(handle) == activeScenes.end());
    activeScenes[handle] = scene;
    return handle;
}

Scene* Application::EraseActiveScene(int handle)
{
    ZoneScopedN("Application::EraseActiveScene");

    auto& iterator = activeScenes.find(handle);
    ASSERT(iterator != activeScenes.end());

    Scene* scene = iterator->second;
    activeScenes.erase(handle);
    return scene;
}

Application::Application(std::string workspacePath)
{
    ZoneScopedN("Application::Application");

    Configuration::Set(CONFIG_WORKSPACE_PATH, workspacePath);

    Logger::Write(
        "Current path: " + std::filesystem::current_path().string(),
        Logger::Level::Info, Logger::MsgType::Platform
    );

    GlfwWindow& window = GlfwWindow::GetInstance();
    renderer::VulkanRenderer& renderer = renderer::VulkanRenderer::GetInstance();
    this->input = Input::GetInstance();
    this->openxr = OpenxrPlatform::Initialize(this->input);
    this->assetManager = new AssetManager();

    window.InitializeWindow();
    renderer.InitializeDevice(
        MergeExtensions(window.GetVkInstanceExt(), 
        openxr? openxr->GetVkInstanceExt(): std::vector<const char*>()), 
        MergeExtensions(window.GetVkDeviceExt(),
        openxr? openxr->GetVkDeviceExt(): std::vector<const char *>()));

    window.InitializeSurface();
    renderer.AllocateResources(window.GetSwapchain(), assetManager);
    assetManager->InitializeWorkspace();

    this->renderer = &renderer;
    this->window = &window;
}

Application::~Application()
{
    ZoneScopedN("Application::~Application");

    if (launchXR)
    {
        renderer->DestroyXrSession();
    }

    assetManager->SaveToFilesystem();
    delete assetManager;

    renderer->DeallocateResources();
    window->DestroySurface();

    renderer->Destroy();
    window->DestroyWindow();

    if (openxr)
    {
        openxr->Destroy();
        delete openxr;
    } 

    renderer = nullptr;
    window = nullptr;
    openxr = nullptr;
}

void Application::PollEvents()
{
    ZoneScopedN("Application::PollEvents");

    if (openxr)
    {
        openxr->PollEvents();

        if(openxr->ShouldCloseSeesion())
            renderer->DestroyXrSession();
    }

    EventQueue::GetInstance()->ProcessEvents();
}

void Application::Run()
{
    ZoneScopedN("Application::Run");

    OnCreated();

    if (launchXR && openxr)
    {
        renderer->InitializeXrSession(openxr->NewSession());
        // TODO: intialization failed
    }
    
    Timer timer{};
    while (!window->ShouldClose()) 
    {
        Timestep ts = timer.GetTimestep();

        PollEvents();
        window->BeginFrame(); // poll window events

        OnUpdated(ts); // outside of scene update loop 

        // Render loop needs to support multiple scenes
        for (auto& scene: activeScenes)
        {   // update loop of a scene
            // if (openxr) openxr->BeginFrame();
            scene.second->Update(ts);
            // if (openxr) openxr->EndFrame();
        }
        renderer->EndFrame();

        FrameMark;
    }
    OnDestroy();
}

std::vector<const char*> Application::MergeExtensions(
    std::vector<const char*> a, std::vector<const char*> b)
{
    ZoneScopedN("Application::MergeExtensions");

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