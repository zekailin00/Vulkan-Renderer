#include "editor.h"

#include "vulkan_renderer.h"
#include "filesystem_viewer.h"
#include "validation.h"
#include "logger.h"

#include <imgui.h>

enum class EditorMenuPopup
{
    None,
    LoadScene
};

static EditorMenuPopup menuPopup;

void Editor::CloseScene(bool saveToFilesystem)
{
    if (this->scene)
    {
        std::string sceneName = this->scene->GetSceneName();
        if (saveToFilesystem)
        {
            this->scene->SaveToFile(assetManager->GetScenePath(sceneName));
            EventWorkspaceChanged* event2 = new EventWorkspaceChanged();
            EventQueue::GetInstance()->Publish(EventQueue::Editor, event2);
        }
        delete application->EraseActiveScene(activeSceneHandle);
        this->scene = nullptr;
        this->activeSceneHandle = -1;
    }
}


Editor::Editor(Application* app)
{
    application = app;

    {
        renderer::TextureBuildInfo info{};
        info.imagePath = "resources/cover_image1.png";
        launcherTexture = std::dynamic_pointer_cast<renderer::VulkanTexture>(
            renderer::VulkanTexture::BuildTexture(&info));

        renderer::VulkanRenderer& vkr = renderer::VulkanRenderer::GetInstance();

        VulkanPipelineLayout& layout = vkr.GetPipelineLayout("display");
        layout.AllocateDescriptorSet("texture", vkr.FRAME_IN_FLIGHT, &launcherDescSet);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = launcherDescSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = launcherTexture->GetDescriptor();

        vkUpdateDescriptorSets(
            vkr.vulkanDevice.vkDevice, 1, &descriptorWrite, 0, nullptr);
    }

    subscriberHandle = EventQueue::GetInstance()->Subscribe(EventQueue::Editor,
        [this](Event* event){
            if (event->type == Event::Type::NewEntity)
            {
                ASSERT(scene != nullptr);

                EventNewEntity* e = reinterpret_cast<EventNewEntity*>(event);
                Entity* entity = scene->NewEntity();
                if (e->parent)
                {
                    entity->ReparentTo(e->parent);
                }
            }
            else if (event->type == Event::Type::DeleteEntity)
            {
                ASSERT(scene != nullptr);
                
                EventDeleteEntity* e = reinterpret_cast<EventDeleteEntity*>(event);
                scene->RemoveEntity(e->entity);
            }
            else if (event->type == Event::Type::DeleteComponent)
            {
                ASSERT(scene != nullptr);
                
                EventDeleteComponent* e =
                    reinterpret_cast<EventDeleteComponent*>(event);
                
                ASSERT(e->entity->HasComponent(e->componentType));
                e->entity->RemoveComponent(e->componentType);
            }
            else if (event->type == Event::Type::CloseProject)
            {
                CloseScene(true);

                EventCloseProject* e = reinterpret_cast<EventCloseProject*>(event);
                ASSERT(e->assetManager == this->assetManager);

                delete this->assetManager;
                this->assetManager = nullptr;
                this->editorState = Editor::EditorState::NoScene;
            }
            else if (event->type == Event::Type::SceneOpen)
            {
                CloseScene(true);

                EventSceneOpen* e = reinterpret_cast<EventSceneOpen*>(event);
                this->scene = reinterpret_cast<Scene*>(e->scene);
                this->activeSceneHandle =
                    this->application->SetActiveScene(this->scene);
                this->editorState = Editor::EditorState::Editing;
            }
            else if (event->type == Event::Type::CloseScene)
            {
                EventCloseScene* e = reinterpret_cast<EventCloseScene*>(event);
                CloseScene(e->saveToFs);
                this->editorState = Editor::EditorState::NoScene;
            }
            else if (event->type == Event::Type::SaveProject)
            {
                if (this->scene)
                {
                    std::string sceneName = this->scene->GetSceneName();
                    this->scene->SaveToFile(assetManager->GetScenePath(sceneName));
                }
                this->assetManager->SaveToFilesystem();
                EventWorkspaceChanged* event2 = new EventWorkspaceChanged();
                EventQueue::GetInstance()->Publish(EventQueue::Editor, event2);
            }
            else if (event->type == Event::Type::SimStart)
            {
                Scene* runningScene = this->scene->Replicate(Scene::State::Running);
                int handle = this->application->SetActiveScene(runningScene);

                if (handle < 0)
                {
                    Logger::Write(
                        "[Editor] Start scene failed: " + std::to_string(handle),
                        Logger::Level::Warning, Logger::MsgType::Editor
                    );

                    delete runningScene;

                    EventSceneSelected* event2 = new EventSceneSelected();
                    event2->scene = this->scene;
                    EventQueue::GetInstance()->Publish(EventQueue::Editor, event2);

                    return;
                };

                CloseScene(true);

                this->scene = runningScene;
                this->activeSceneHandle = handle;
                this->editorState = EditorState::Running;

                EventSceneSelected* event2 = new EventSceneSelected();
                event2->scene = runningScene;
                EventQueue::GetInstance()->Publish(EventQueue::Editor, event2);
            }
            else if (event->type == Event::Type::SimStartVR)
            {
                Scene* runningScene = this->scene->Replicate(Scene::State::RunningVR);
                int handle = this->application->SetActiveScene(runningScene);

                if (handle < 0)
                {
                    Logger::Write(
                        "[Editor] Start scene failed: " + std::to_string(handle),
                        Logger::Level::Warning, Logger::MsgType::Editor
                    );

                    delete runningScene;

                    EventSceneSelected* event2 = new EventSceneSelected();
                    event2->scene = this->scene;
                    EventQueue::GetInstance()->Publish(EventQueue::Editor, event2);

                    return;
                };

                CloseScene(true);

                this->scene = runningScene;
                this->activeSceneHandle = handle;
                this->editorState = EditorState::Running;

                EventSceneSelected* event2 = new EventSceneSelected();
                event2->scene = runningScene;
                EventQueue::GetInstance()->Publish(EventQueue::Editor, event2);
            }
            else if (event->type == Event::Type::SimStop)
            {
                std::string sceneName = this->scene->GetSceneName();
                CloseScene(false);

                std::string strPath = assetManager->GetScenePath(sceneName);

                this->scene = Scene::LoadFromFile(
                    strPath, assetManager, Scene::State::Editor
                );

                this->activeSceneHandle =
                    this->application->SetActiveScene(this->scene);

                this->editorState = EditorState::Editing;

                EventSceneSelected* event2 = new EventSceneSelected();
                event2->scene = scene;
                EventQueue::GetInstance()->Publish(EventQueue::Editor, event2);
            }
        });
}

Editor::~Editor()
{
    EventQueue::GetInstance()->Unsubscribe(subscriberHandle);
    launcherTexture = nullptr; //free smart pointer
}

void Editor::DrawPopups()
{
    if (menuPopup == EditorMenuPopup::LoadScene) ImGui::OpenPopup("Load Scene");

    if (ImGui::BeginPopupModal("Load Scene", NULL,
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
    {
        menuPopup = EditorMenuPopup::None;

        ImGui::Text("Load a scene from the workspace.");
        ImGui::Separator();

        static std::vector<std::string> avaiableScenes;
        static bool sceneLoaded = false;
        if (!sceneLoaded)
        {
            assetManager->GetAvailableScenes(avaiableScenes);
            sceneLoaded = true;
        }

        static const char* currenScenePath = "None";
        if (ImGui::BeginCombo("Scene Path", currenScenePath))
        {
            for (int n = 0; n < avaiableScenes.size(); n++)
            {
                if (scene)
                {
                    std::filesystem::path path = avaiableScenes[n];
                    std::string s0 = path.stem().string();
                    std::string s1 = scene->GetSceneName();
                    if (s0 == s1)
                        continue;
                }
                
                const bool isSelected =
                    strcmp(avaiableScenes[n].c_str(), currenScenePath) == 0;
                if (ImGui::Selectable(avaiableScenes[n].c_str(), isSelected))
                {
                    currenScenePath = avaiableScenes[n].c_str();
                }

                // Set the initial focus to the selected mesh
                // when opening the combo
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            Scene* scene = Scene::LoadFromFile(
                assetManager->GetWorkspacePath() + "/" + currenScenePath,
                assetManager, Scene::State::Editor
            );

            EventSceneOpen* event = new EventSceneOpen();
            event->scene = scene;
            EventQueue::GetInstance()->Publish(EventQueue::Editor, event);

            sceneLoaded = false;
            currenScenePath = "None";
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            sceneLoaded = false;
            currenScenePath = "None";
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void Editor::DrawUI()
{
    if (!assetManager)
    {
        DrawLauncher();
        return;
    }

    ImGuiID dockID = ImGui::DockSpaceOverViewport();
    DrawMenu();
    DrawPopups();
    viewport.Draw(dockID);
    sceneGraph.Draw();
    entityProperties.Draw();
    workspace.Draw();
    materialEditor.Draw();
    textureEditor.Draw();
    ImGui::ShowDemoWindow();
}

void Editor::DrawLauncher()
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos({0.0f, 0.0f});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 1.0f)); 
    ImGui::Begin("window", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus
    );

    ImGui::Image(launcherDescSet, {
        io.DisplaySize.x * 0.7f, io.DisplaySize.y * 0.7f
    });

    ImGui::End();
    ImGui::PopStyleColor();


    ImGui::Begin("Open a project...", 0,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);

    static char strPath[128] = "";
    ImGui::InputTextWithHint("directory path",
        "Path to a project directory", strPath, IM_ARRAYSIZE(strPath));


    if (ImGui::Button("New"))
    {
        if (assetManager = AssetManager::NewProject(strPath))
        {
            EventProjectOpen* event = new EventProjectOpen();
            event->assetManager = assetManager;
            EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Open"))
    {
        if (assetManager = AssetManager::OpenProject(strPath))
        {
            EventProjectOpen* event = new EventProjectOpen();
            event->assetManager = assetManager;
            EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
        }
    }

    ImGui::End();
    ImGui::ShowDemoWindow();
}

void Editor::DrawMenu()
{
    // Top menu
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene", nullptr, false,
                editorState == EditorState::Editing | 
                editorState == EditorState::NoScene))
            {
                std::string strPath =
                    Filesystem::GetUnusedFilePath(assetManager->GetScenePath("scene"));

                std::filesystem::path fsPath = strPath;
                Scene* scene = Scene::NewScene(fsPath.stem().string(), assetManager);
                scene->SaveToFile(assetManager->GetScenePath(fsPath.stem().string()));

                EventSceneOpen* event = new EventSceneOpen();
                event->scene = scene;
                EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
            }

            if (ImGui::MenuItem("Load Scene", nullptr, false,
                editorState == EditorState::Editing |
                editorState == EditorState::NoScene))
            {
                menuPopup = EditorMenuPopup::LoadScene;
            }

            if (ImGui::MenuItem("Close Scene", nullptr, false,
                editorState == EditorState::Editing))
            {
                EventCloseScene* event = new EventCloseScene();
                event->saveToFs = false;
                EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
            }

            if (ImGui::MenuItem("Close and Save Scene", nullptr, false,
                editorState == EditorState::Editing))
            {
                EventCloseScene* event = new EventCloseScene();
                event->saveToFs = true;
                EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
            }

            if (ImGui::MenuItem("Close Project", nullptr, false,
                editorState == EditorState::Editing|
                editorState == EditorState::NoScene))
            {
                EventCloseProject* event = new EventCloseProject();
                event->assetManager = this->assetManager;
                EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Save", nullptr, false,
                editorState == EditorState::Editing))
            {
                EventSaveProject* event = new EventSaveProject();
                EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
            }

            if (ImGui::MenuItem("Undo", nullptr, false))
            {

            }

            if (ImGui::MenuItem("Redo", nullptr, false))
            {

            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Run"))
        {
            if (ImGui::MenuItem("Start", nullptr, false,
                editorState == EditorState::Editing))
            {
                EventSimStart* event = new EventSimStart();
                EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
            }

            if (ImGui::MenuItem("Start in VR", nullptr, false,
                editorState == EditorState::Editing))
            {
                EventSimStartVR* event = new EventSimStartVR();
                EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
            }

            if (ImGui::MenuItem("Pause", nullptr, false))
            {

            }

            if (ImGui::MenuItem("Continue", nullptr, false))
            {

            }

            if (ImGui::MenuItem("Stop", nullptr, false,
                editorState == EditorState::Running))
            {
                EventSimStop* event = new EventSimStop();
                EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Scene Graph", nullptr, false))
            {
            
            }

            if (ImGui::MenuItem("Workspace", nullptr, false))
            {

            }

            if (ImGui::MenuItem("Viewport", nullptr, false))
            {
                
            }

            if (ImGui::MenuItem("Entity Properties", nullptr, false))
            {
                
            }

            if (ImGui::MenuItem("Material Editor", nullptr, false))
            {
                
            }

            if (ImGui::MenuItem("Texture Editor", nullptr, false))
            {
                
            }

            if (ImGui::MenuItem("ImGui Demo", nullptr, false))
            {
                
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}
