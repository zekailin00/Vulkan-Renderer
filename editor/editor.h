#pragma once

#include "application.h"

class Editor
{
public:
    void operator()()
    {
        ImGui::Text("test test test test");
        ImGui::Text("test test test test");
        ImGui::Text("test test test test");
        ImGui::Text("test test test test");
        ImGui::ShowDemoWindow();
    }
};