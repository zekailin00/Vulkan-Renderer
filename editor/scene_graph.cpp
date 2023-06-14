#include "scene_graph.h"

#include "events.h"

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
        ShowEntityChildren(scene->GetRootEntity()->GetChildren(), &selectedEntity);
    }

    ImGui::End();
}

void SceneGraph::ShowEntityChildren(const std::list<Entity*>& children, Entity** selected)
{
    const static ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    for (Entity* entity: children)
    {
        ImGuiTreeNodeFlags nodeFlags = treeFlags;
        if (entity == *selected)
            nodeFlags |= ImGuiTreeNodeFlags_Selected;

        if(!entity->GetChildren().empty())
        {
            bool isTreeOpen = ImGui::TreeNodeEx(entity->name.c_str(), nodeFlags);
            bool isPopupOpen = ImGui::BeginPopupContextItem();

            if (isPopupOpen)
            {
                *selected = entity;
                ShowEntityPopupContext(entity);
            }

            if (ImGui::IsItemClicked())
            {
                *selected = entity;
            }

            if (isTreeOpen)
            {
                ShowEntityChildren(entity->GetChildren(), selected);
                ImGui::TreePop();
            }
        }
        else
        {
            nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            ImGui::TreeNodeEx(entity->name.c_str(), nodeFlags);
            bool isPopupOpen = ImGui::BeginPopupContextItem();

            if (isPopupOpen)
            {
                *selected = entity;
                ShowEntityPopupContext(entity);
            }

            if (ImGui::IsItemClicked())
            {
                *selected = entity;
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