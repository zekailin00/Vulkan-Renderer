#include "scene_graph.h"

#include "events.h"

SceneGraph::SceneGraph()
{
    subscriberHandle = EventQueue::GetInstance()->Subscribe(EventQueue::Editor,
        [this](Event* event){
            if (event->type == Event::Type::EntitySelected)
            {
                EventEntitySelected* e = dynamic_cast<EventEntitySelected*>(event);
                this->selectedEntity = e->entity;
            }
            else if (event->type == Event::Type::CloseProject)
            {
                this->scene = nullptr;
                this->selectedEntity = nullptr;
            }
            else if (event->type == Event::Type::SceneOpen)
            {
                EventSceneOpen* e = reinterpret_cast<EventSceneOpen*>(event);
                this->scene = reinterpret_cast<Scene*>(e->scene);
            }
            else if (event->type == Event::Type::CloseScene)
            {
                this->scene = nullptr;
                this->selectedEntity = nullptr;
            }
            else if (event->type == Event::Type::SceneSelected)
            {
                EventSceneSelected* e = reinterpret_cast<EventSceneSelected*>(event);
                this->scene = reinterpret_cast<Scene*>(e->scene);
            }
            else if (event->type == Event::Type::SimStart)
            {
                this->scene = nullptr;
                this->selectedEntity = nullptr;
            }
            else if (event->type == Event::Type::SimStartVR)
            {
                this->scene = nullptr;
                this->selectedEntity = nullptr;
            }
            else if (event->type == Event::Type::SimStop)
            {
                this->scene = nullptr;
                this->selectedEntity = nullptr;
            }
        });
}

SceneGraph::~SceneGraph()
{
    EventQueue::GetInstance()->Unsubscribe(subscriberHandle);
}

void SceneGraph::SetScene(Scene* scene)
{
    this->scene = scene;
}

void SceneGraph::Draw()
{
    ImGui::Begin("Scene Graph", nullptr);

    ImGui::TextWrapped("A list of entities in a scene.");
    ImGui::Separator();

    if (scene)
    {
        ImGui::Text("%s",scene->GetSceneName().c_str());
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 70);
        if (ImGui::SmallButton("New Entity"))
        {
            PublishNewEntityEvent(nullptr);
        }

        ShowEntityChildren(scene->GetRootEntity()->GetChildren());
    }
    else
    {
        ImGui::Text("No open scene.");
    }

    ImGui::End();
}

void SceneGraph::ShowEntityChildren(const std::list<Entity*>& children)
{
    const static ImGuiTreeNodeFlags treeFlags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    for (Entity* entity: children)
    {
        ImGuiTreeNodeFlags nodeFlags = treeFlags;
        if (entity == selectedEntity)
            nodeFlags |= ImGuiTreeNodeFlags_Selected;

        if(!entity->GetChildren().empty())
        {
            bool isTreeOpen = ImGui::TreeNodeEx(entity->GetName().c_str(), nodeFlags);
            bool isPopupOpen = ImGui::BeginPopupContextItem();

            if (isPopupOpen)
            {
                PublishEntitySelectedEvent(entity);
                ShowEntityPopupContext(entity);
            }

            if (ImGui::IsItemClicked())
            {
                PublishEntitySelectedEvent(entity);
            }

            if (isTreeOpen)
            {
                ShowEntityChildren(entity->GetChildren());
                ImGui::TreePop();
            }
        }
        else
        {
            nodeFlags |= ImGuiTreeNodeFlags_Leaf |
                         ImGuiTreeNodeFlags_NoTreePushOnOpen;
            ImGui::TreeNodeEx(entity->GetName().c_str(), nodeFlags);
            bool isPopupOpen = ImGui::BeginPopupContextItem();

            if (isPopupOpen)
            {
                PublishEntitySelectedEvent(entity);
                ShowEntityPopupContext(entity);
            }

            if (ImGui::IsItemClicked())
            {
                PublishEntitySelectedEvent(entity);
            }
        }
    }
} 

void SceneGraph::ShowEntityPopupContext(Entity* entity)
{
    if(ImGui::Button("New Child"))
    {
        PublishNewEntityEvent(entity);
        ImGui::CloseCurrentPopup();
    }
    if(ImGui::Button("Delete"))
    {
        PublishDeleteEntityEvent(entity);
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void SceneGraph::PublishEntitySelectedEvent(Entity* entity)
{
    EventEntitySelected* event = new EventEntitySelected();
    event->entity = entity;

    EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
}

void SceneGraph::PublishNewEntityEvent(Entity* parent)
{
    EventNewEntity* event = new EventNewEntity();
    event->parent = parent;

    EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
}

void SceneGraph::PublishDeleteEntityEvent(Entity* entity)
{
    EventDeleteEntity* event = new EventDeleteEntity();
    event->entity = entity;

    EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
}