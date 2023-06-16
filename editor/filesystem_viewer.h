#pragma once

#include "filesystem.h"
#include "validation.h"

#include <string>
#include <filesystem>
#include <imgui.h>

/**
 * @brief This is currently broken
 * 
 */
class FilesystemViewer
{
public:
    static void Draw(std::filesystem::path directory, std::string& selectedPath)
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
            ImGui::TableHeadersRow();

            DrawDirectory(directory, selectedPath);
        
            ImGui::EndTable();
        }
    }
private:
    static void DrawDirectory(std::filesystem::path directory, std::string& selectedPath)
    {
        std::vector<std::filesystem::path> entries;
        Filesystem::GetDirectoryEntries(directory.string(), entries);
        for (auto& p: entries)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            std::string entryName;
            bool isDirectory;
            try
            {
                entryName = p.stem().generic_string();
                isDirectory = Filesystem::IsDirectory(p.string());
            }
            catch(...)
            {
                entryName = "Invalid name encoding";
                isDirectory = false;
            }

            bool open = ImGui::TreeNodeEx(entryName.c_str(),
                isDirectory?0: ImGuiTreeNodeFlags_Leaf |
                ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen
            );
            ImGui::TableNextColumn();
            ImGui::TextDisabled("--");
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(isDirectory?"Folder":"File");
     
            if (open)
            {
                if(isDirectory)
                    DrawDirectory(p, selectedPath);
                ImGui::TreePop();
            }

        }
    }
};