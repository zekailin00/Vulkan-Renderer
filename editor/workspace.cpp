#include "workspace.h"

#include "configuration.h"
#include "filesystem.h"
#include "validation.h"
#include "event_queue.h"
#include "events.h"

#include "scripting_subsystem.h"

enum class WorkspacePopup
{
    None,
    ImportTexture,
    ImportOBJ,
    ImportGLTF
};

static WorkspacePopup workspacePopup;

Workspace::Workspace()
{
    subscriberHandle = EventQueue::GetInstance()->Subscribe(EventQueue::Editor,
    [this](Event* event){
        if (event->type == Event::Type::ProjectOpen)
        {
            EventProjectOpen* e = dynamic_cast<EventProjectOpen*>(event);
            this->assetManager = reinterpret_cast<AssetManager*>(e->assetManager);
        }
        else if (event->type == Event::Type::CloseProject)
        {
            this->root = nullptr;
            this->filesystemCache = false;
            this->assetManager = nullptr;
            this->scene = nullptr;
        }
        else if (event->type == Event::Type::SceneOpen)
        {
            EventSceneOpen* e = reinterpret_cast<EventSceneOpen*>(event);
            this->scene = reinterpret_cast<Scene*>(e->scene);
        }
        else if (event->type == Event::Type::CloseScene)
        {
            this->scene = nullptr;
        }
        else if (event->type == Event::Type::WorkspaceChanged)
        {
            this->filesystemCache = false;
        }
        else if (event->type == Event::Type::SceneSelected)
        {
            EventSceneSelected* e = reinterpret_cast<EventSceneSelected*>(event);
            this->scene = reinterpret_cast<Scene*>(e->scene);
        }
        else if (event->type == Event::Type::SimStart)
        {
            this->scene = nullptr;
        }
        else if (event->type == Event::Type::SimStartVR)
        {
            this->scene = nullptr;
        }
        else if (event->type == Event::Type::SimStop)
        {
            this->scene = nullptr;
        }
    });
}

Workspace::~Workspace()
{
    EventQueue::GetInstance()->Unsubscribe(subscriberHandle);
}


void Workspace::Draw()
{
    ASSERT(assetManager != nullptr);
    ImGui::Begin("Workspace", nullptr);

    DrawButtons();
    ImGui::Separator();
    DrawTable();
    DrawPopups();


    ImGui::End(); 
}

void Workspace::DrawButtons()
{
    if(ImGui::Button("Refresh"))
    {
        EventWorkspaceChanged* event = new EventWorkspaceChanged();
        EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
    }
    ImGui::SameLine();
    if(ImGui::Button("Add"))
    {
        ImGui::OpenPopup("Add Resources");
    }
    ImGui::SameLine();
    if(ImGui::Button("Open VS Code"))
    {

    }

    //------- Add action popups -------//

    if (ImGui::BeginPopup("Add Resources", ImGuiWindowFlags_NoMove))
    {
        if (ImGui::BeginMenu("Import..."))
        {
            if (ImGui::MenuItem("Texture"))
            {
                workspacePopup = WorkspacePopup::ImportTexture;
            }

            if (ImGui::BeginMenu("Model"))
            {
                if (ImGui::MenuItem(".glfw"))
                {
                    workspacePopup = WorkspacePopup::ImportGLTF;
                }

                if (ImGui::MenuItem(".obj"))
                {
                    workspacePopup = WorkspacePopup::ImportOBJ;
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("New Material"))
        {
            assetManager->NewMaterial();
            EventWorkspaceChanged* event = new EventWorkspaceChanged();
            EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
        }

        if (ImGui::MenuItem("New Script"))
        {
            std::filesystem::path path = Filesystem::GetUnusedFilePath(
                assetManager->GetScriptPath("source")
            );

            std::string fileName = path.stem().string();
            assetManager->NewSourceCode(fileName);

            EventWorkspaceChanged* event = new EventWorkspaceChanged();
            EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
        }

        ImGui::EndPopup();
    }
}

void Workspace::DrawPopups()
{
    if (workspacePopup == WorkspacePopup::ImportTexture)
        ImGui::OpenPopup("Import Texture");
    if (workspacePopup == WorkspacePopup::ImportOBJ)
        ImGui::OpenPopup("Import Model: .obj");
    if (workspacePopup == WorkspacePopup::ImportGLTF)
        ImGui::OpenPopup("Import Model: .gltf");
    

    // Import Texture popup modal window
    if (ImGui::BeginPopupModal("Import Texture", NULL,
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
    {
        workspacePopup = WorkspacePopup::None;
        static char strPath[128] = "";
        static bool okFailed = false;

        ImGui::Text("Import an image into the workspace.");
        ImGui::Separator();

        ImGui::InputTextWithHint("texture path",
            "Absolute path to an image", strPath, IM_ARRAYSIZE(strPath));

        // Ok or cancel. Resets static value.
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            if(assetManager->ImportTexture(strPath) == nullptr)
            {
                okFailed = true;
            }
            else
            {
                EventWorkspaceChanged* event = new EventWorkspaceChanged();
                EventQueue::GetInstance()->Publish(EventQueue::Editor, event);

                okFailed = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            okFailed = false;
            ImGui::CloseCurrentPopup();
        }

        // Message when Ok button failed.
        if (okFailed)
        {
            ImGui::Text("Path is invalid.");
        }

        ImGui::EndPopup();
    }


    // Import .obj model popup modal window
    if (ImGui::BeginPopupModal("Import Model: .obj", NULL,
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
    {
        workspacePopup = WorkspacePopup::None;
        static char strPath[128] = "";
        static bool okFailed = false;

        ImGui::Text("Import a .obj model into the workspace.");
        ImGui::Separator();

        ImGui::InputTextWithHint("Model path",
            "Absolute path to a model", strPath, IM_ARRAYSIZE(strPath));

        // Ok or cancel. Resets static value.
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            if(!scene || assetManager->ImportModelObj(strPath, scene) == nullptr)
            {
                okFailed = true;
            }
            else
            {
                EventWorkspaceChanged* event = new EventWorkspaceChanged();
                EventQueue::GetInstance()->Publish(EventQueue::Editor, event);

                okFailed = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            okFailed = false;
            ImGui::CloseCurrentPopup();
        }

        // Message when Ok button failed.
        if (okFailed)
        {
            ImGui::Text("Import failed: Invalid path or no scene open.");
        }

        ImGui::EndPopup();
    }

    // Import .obj model popup modal window
    if (ImGui::BeginPopupModal("Import Model: .gltf", NULL,
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
    {
        workspacePopup = WorkspacePopup::None;
        static char strPath[128] = "";
        static bool okFailed = false;

        ImGui::Text("Import a .gltf model into the workspace.");
        ImGui::Separator();

        ImGui::InputTextWithHint("Model path",
            "Absolute path to a model", strPath, IM_ARRAYSIZE(strPath));

        // Ok or cancel. Resets static value.
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            if(!scene || assetManager->ImportModelGltf(strPath, scene) == nullptr)
            {
                okFailed = true;
            }
            else
            {
                EventWorkspaceChanged* event = new EventWorkspaceChanged();
                EventQueue::GetInstance()->Publish(EventQueue::Editor, event);

                okFailed = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            okFailed = false;
            ImGui::CloseCurrentPopup();
        }

        // Message when Ok button failed.
        if (okFailed)
        {
            ImGui::Text("Import failed: Invalid path or no scene open.");
        }

        ImGui::EndPopup();
    }
}

void Workspace::DrawTable()
{
    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;

    ImGuiTableFlags flags =
        ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | 
        ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_NoBordersInBody;

    if (ImGui::BeginTable("4ways", 4, flags))
    {
        ImGui::TableSetupColumn(
            "Name",ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn
        ("Size", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
        ImGui::TableSetupColumn(
            "Type", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
        ImGui::TableSetupColumn(
            "Path", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 25.0f);
        ImGui::TableHeadersRow();

        if (!filesystemCache)
        {
            std::string workspacePath = assetManager->GetWorkspacePath();
            std::filesystem::path fsPath = workspacePath;

            root = std::make_unique<FileSystemNode>();
            root->path = "";
            root->name = fsPath.stem().string();
            root->isDirectory = true;
            BuildNodes(root);
            filesystemCache = true;
        }

        RenderNodes(root);
        ImGui::EndTable();
    }
}

void Workspace::BuildNodes(std::unique_ptr<FileSystemNode>& node)
{
    std::string workspacePath = assetManager->GetWorkspacePath();

    for (auto const& entry : std::filesystem::directory_iterator{
        workspacePath + "/" + node->path.string()}) 
    {
        std::unique_ptr<FileSystemNode> childNode =
            std::make_unique<FileSystemNode>();
        childNode->name = entry.path().filename().string();
        childNode->isDirectory = entry.is_directory();
        childNode->size = childNode->isDirectory?0:entry.file_size();
        childNode->path = Filesystem::RemoveParentPath(
            entry.path().string(), workspacePath);

        if (childNode->isDirectory)
            BuildNodes(childNode);
        node->children.push_back(std::move(childNode));
    }
}

void Workspace::RenderNodes(std::unique_ptr<FileSystemNode>& node)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    if (node->isDirectory)
    {
        bool open = ImGui::TreeNodeEx(node->name.c_str(),
            ImGuiTreeNodeFlags_SpanFullWidth);
        ImGui::TableNextColumn();
        ImGui::TextDisabled("--");
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(node->isDirectory?"Folder":"File");
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(node->path.string().c_str());
        if (open)
        {
            for (int i = 0; i < node->children.size(); i++)
                RenderNodes(node->children[i]);
            ImGui::TreePop();
        }
    }
    else
    {
        ImGui::TreeNodeEx(node->name.c_str(),
            ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet |
            ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);

        ImGui::TableNextColumn();
        ImGui::Text("%d", node->size);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(node->isDirectory?"Folder":"File");
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(node->path.string().c_str());
    }
}