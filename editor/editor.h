#pragma once

#include "application.h"

#include "viewport.h"
#include "scene_graph.h"
#include "workspace.h"
#include "entity_properties.h"
#include "material_editor.h"
#include "texture_editor.h"


class Editor
{

private:
    Viewport viewport;
    SceneGraph sceneGraph;
    EntityProperties entityProperties;
    Workspace workspace;
    MaterialEditor materialEditor;
    TextureEditor textureEditor;

    Scene* project;
public:

    Editor(const Editor& editor)
    {
        project = editor.project;
        viewport.SetScene(editor.project);
        sceneGraph.SetScene(editor.project);
    }

    const Editor& operator=(const Editor& editor)
    {
        project = editor.project;
        viewport.SetScene(editor.project);
        sceneGraph.SetScene(editor.project);
    }

    Editor(Scene* scene)
    {
        project = scene;
        viewport.SetScene(scene);
        sceneGraph.SetScene(scene);
    }

    void operator()()
    {
        ImGuiID dockID = ImGui::DockSpaceOverViewport();
        Draw();
        viewport.Draw(dockID);
        sceneGraph.Draw();
        entityProperties.Draw();
        workspace.Draw();
        materialEditor.Draw();
        textureEditor.Draw();
        ImGui::ShowDemoWindow();
    }

    void Draw()
    {
        // Top menu
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Entity", nullptr, false))
                {

                }
                if (ImGui::MenuItem("Save and close", nullptr, false))
                {

                }
                if (ImGui::MenuItem("Close", nullptr, false))
                {

                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Save", nullptr, false))
                {

                }

                if (ImGui::MenuItem("Load Entity from", nullptr, false))
                {

                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Run"))
            {
                if (ImGui::MenuItem("Start", nullptr, false))
                {
                
                }

                if (ImGui::MenuItem("Pause", nullptr, false))
                {

                }

                if (ImGui::MenuItem("Continue", nullptr, false))
                {

                }

                if (ImGui::MenuItem("Stop", nullptr, false))
                {

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
                if (ImGui::MenuItem("Component List", nullptr, false))
                {
                    
                }

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
};