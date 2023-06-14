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
        ShowEntityChildren(scene->GetRootEntity()->GetChildren());
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
    }
    if(ImGui::Button("Delete"))
    {
        PublishDeleteEntityEvent(entity);
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