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