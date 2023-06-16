#include "workspace.h"

#include "configuration.h"
#include "filesystem.h"
#include "validation.h"
#include "event_queue.h"
#include "events.h"


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
        }
        else if (event->type == Event::Type::WorkspaceChanged)
        {
            this->filesystemCache = false;
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


    ImGui::End(); 
}

void Workspace::DrawButtons()
{
    if(ImGui::Button("Refresh"))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("Import"))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("Open VS Code"))
    {

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