#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "logger.h"


class Filesystem
{
public:
    
    /**
     * @brief Check if the path points to a directory.
     * 
     * @param path An absolute path.
     * @return Return true if the absolute path points to a directory.
     */
    static bool IsDirectory(std::string path);

    /**
     * @brief Check if the path points to a file.
     * 
     * @param path An absolute path.
     * @return Return true if the absolute path points to a file. 
     */
    static bool IsRegularFile(std::string path);

    /**
     * @brief Get the directory entries
     * 
     * @param path A path to the directory.
     * @param entries Entries in the directory are written to this container.
     * @return Returns false if the path does not point to a directory.
     */
    static bool GetDirectoryEntries(
        std::string path, std::vector<std::filesystem::path>& entries);

    /**
     * @brief Create a directory in the file system.
     * 
     * @param path An absolute path.
     * @return Returns true if at least one directory is created.
     * Return false if no new directory is created,
     * or the path is not a relative path.
     */
    static bool CreateDirectory(std::string path);

    static bool GetRelativePath(
        std::string fullPath, std::string parentPath,
        std::string& relativePath);

    static std::string ChangeExtensionTo(std::string path, std::string newExtension);

    static std::string RemoveParentPath(std::string fullPath, std::string parentPath);

    static std::string GetUnusedFilePath(std::string path);

    static void ToUnixPath(std::string& path);

    static bool IsAbsolute(std::filesystem::path absolutePath);
};