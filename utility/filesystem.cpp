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

bool Filesystem::IsAbsolute(std::filesystem::path absolutePath)
{
    if(absolutePath.is_absolute())
        return true;

    Logger::Write(
        absolutePath.string() + " is not an absolute path",
        Logger::Level::Warning, Logger::Platform
    );
    return false;
}