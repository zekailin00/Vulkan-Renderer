#include "filesystem.h"


bool Filesystem::IsDirectory(std::string path)
{
    if (!IsAbsolute(path))
        return false;
    return std::filesystem::is_directory(path);
}

bool Filesystem::IsRegularFile(std::string path)
{
    if (!IsAbsolute(path))
        return false;
    return std::filesystem::is_regular_file(path);
}

bool Filesystem::GetDirectoryEntries(
    std::string path, std::vector<std::filesystem::path>& entries)
{
    entries.clear();
    
    if (!IsAbsolute(path))
        return false;

    if (!IsDirectory(path)) 
        return false;

    for (auto const& entry: std::filesystem::directory_iterator{path}) 
        entries.push_back(entry.path());
    
    return true;
}

bool Filesystem::CreateDirectory(std::string path)
{
    if (!IsAbsolute(path))
        return false;

    bool result = std::filesystem::create_directories(path);
    return result;
}

bool Filesystem::GetRelativePath(
    std::string fullPath, std::string parentPath, std::string& relativePath)
{
    relativePath = "";

    if (!IsAbsolute(fullPath) || !IsAbsolute(parentPath))
        return false;

    if (fullPath.substr(0, parentPath.size()) != parentPath)
    {
        Logger::Write(
            parentPath + " is not a part of " + fullPath,
            Logger::Level::Warning, Logger::MsgType::Platform
        );
        return false;
    }

    relativePath = fullPath.substr(parentPath.size() + 1);
    return true;
}

std::string Filesystem::ChangeExtensionTo(std::string path, std::string newExtension)
{
    std::filesystem::path fsPath = path;
    return path.substr(0, path.size() - fsPath.extension().string().size()) + newExtension;
}

std::string Filesystem::RemoveParentPath(std::string fullPath, std::string parentPath)
{
    return fullPath.substr(parentPath.size() + 1);
}

std::string Filesystem::GetUnusedFilePath(std::string path)
{
    std::filesystem::path fsPath = path;
    
    std::string extension = fsPath.extension().string();
    std::string stem = fsPath.stem().string();
    std::string parent = path.substr(0,
        path.size() - extension.size() - stem.size());

    std::string finalPath = parent + stem + extension;
    if (IsRegularFile(finalPath))
    {
        int fileCount = 1;
        do {
            finalPath = parent + stem + std::to_string(fileCount) + extension;
        } while (IsRegularFile(finalPath));
    }

    return finalPath;
}

bool Filesystem::IsAbsolute(std::filesystem::path absolutePath)
{
    if(absolutePath.is_absolute())
        return true;

    Logger::Write(
        absolutePath.string() + " is not an absolute path",
        Logger::Level::Warning, Logger::MsgType::Platform
    );
    return false;
}