#pragma once

#include "application.h"

#include <filesystem>

class Workspace
{
private:
    struct FileSystemNode
    {
        std::string name;
        bool isDirectory;
        uint32_t size;
        std::filesystem::path path; // Path relative to workspace path
        std::vector<std::unique_ptr<FileSystemNode>> children;
    };

public:
    Workspace();
    ~Workspace();
    
    void Draw();

private:
    void DrawButtons();
    void DrawTable();

    void BuildNodes(std::unique_ptr<FileSystemNode>& node);
    void RenderNodes(std::unique_ptr<FileSystemNode>& node);

private:
    int subscriberHandle = -1;

    std::unique_ptr<FileSystemNode> root = nullptr;
    bool filesystemCache = false;

    AssetManager* assetManager = nullptr;
};