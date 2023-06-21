#include <iostream>
#include <filesystem>
#include <memory>

#include "application.h"
#include "logger.h"
#include "timestep.h"
#include "validation.h"

#include "openxr_components.h"


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

    if (scene->GetState() == Scene::State::RunningVR)
    {
        if (!openxr->SystemFound())
        {
            return SCENE_XR_SYSTEM_NOT_FOUNT;
        }

        std::shared_ptr<renderer::VulkanVrDisplay> vrDisplay = nullptr;
        int displayCount = 0;

        scene->GetRootEntity()->ScanEntities(
            [&vrDisplay, &displayCount](Entity* entity){
                if (entity->HasComponent(Component::Type::VrDisplay))
                {
                    displayCount++;
                    vrDisplay = ((renderer::VrDisplayComponent*)entity
                        ->GetComponent(Component::Type::VrDisplay))
                        ->vrDisplay; 
                }
            }
        );

        if (displayCount > 1 || vrDisplay == nullptr)
        {
            return SCENE_VALIDATION_FAILED;
        }

        // Renderer initializes the swapchain
        // vrDisplay then is initialized with the correct extent
        OpenxrSession* session = openxr->NewSession();
        renderer->InitializeXrSession(session);
        vrDisplay->Initialize({
            // Software antialising by a factor of 4
            session->GetWidth() * 4,
            session->GetHeight() * 4
        });

        renderer->SetXRWindowContext(vrDisplay);
    }

    int handle = activeSceneHandleCount++;
    ASSERT(scene != nullptr);
    ASSERT(activeScenes.find(handle) == activeScenes.end());
    activeScenes[handle] = scene;
    return handle;
}

Scene* Application::EraseActiveScene(int handle)
{
    ZoneScopedN("Application::EraseActiveScene");

    auto iterator = activeScenes.find(handle);
    ASSERT(iterator != activeScenes.end());

    Scene* scene = iterator->second;
    activeScenes.erase(handle);

    if (scene->GetState() == Scene::State::RunningVR &&
        openxr->IsSessionRunning())
    {
        openxr->RequestCloseSession();
        while(!openxr->ShouldCloseSession())
        {
            openxr->PollEvents();
        }
        renderer->DestroyXrSession();
    }

    return scene;
}

Application::Application()
{
    ZoneScopedN("Application::Application");

    Logger::Write(
        "Current path: " + std::filesystem::current_path().string(),
        Logger::Level::Info, Logger::MsgType::Platform
    );

    this->window = &GlfwWindow::GetInstance();
    this->renderer = &renderer::VulkanRenderer::GetInstance();
    this->input = Input::GetInstance();
    this->openxr = OpenxrPlatform::Initialize();
    this->eventQueue = EventQueue::GetInstance();

    window->InitializeWindow();
    renderer->InitializeDevice(
        MergeExtensions(window->GetVkInstanceExt(), openxr->GetVkInstanceExt()), 
        MergeExtensions(window->GetVkDeviceExt(), openxr->GetVkDeviceExt()));
    window->InitializeSurface();
    renderer->AllocateResources(window->GetSwapchain());
}

Application::~Application()
{
    ZoneScopedN("Application::~Application");

    renderer->DeallocateResources();
    window->DestroySurface();

    openxr->Destroy();
    delete openxr;

    renderer->Destroy();
    window->DestroyWindow();

    renderer = nullptr;
    window = nullptr;
    openxr = nullptr;
    eventQueue = nullptr;
    input = nullptr;
}

void Application::PollEvents()
{
    ZoneScopedN("Application::PollEvents");

    openxr->PollEvents();

    if(openxr->ShouldCloseSession())
        renderer->DestroyXrSession();

    eventQueue->ProcessEvents();
    window->BeginFrame(); // poll window events
}

void Application::Run()
{
    ZoneScopedN("Application::Run");

    OnCreated();
    
    Timer timer{};
    while (!window->ShouldClose()) 
    {
        Timestep ts = timer.GetTimestep();

        PollEvents();

        OnUpdated(ts); // outside of scene update loop 

        // Render loop needs to support multiple scenes
        bool hasVR = false;
        for (auto& scene: activeScenes)
        {   
            // update loop of a scene
            if (scene.second->GetState() == Scene::State::Editor)
            {
                scene.second->Update(ts);
            }
            else if (scene.second->GetState() == Scene::State::Running)
            {
                scene.second->Update(ts);
            }
            else if (scene.second->GetState() == Scene::State::RunningVR)
            {
                openxr->BeginFrame();
                scene.second->Update(ts);
                hasVR = true;
            }
        }
        renderer->EndFrame();
        if (hasVR) openxr->EndFrame();

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