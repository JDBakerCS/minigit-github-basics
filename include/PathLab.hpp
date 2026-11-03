#pragma once
#include <filesystem>
#include <string>

namespace minigit
{

    std::filesystem::path headFilePath(const std::filesystem::path &root);

    std::filesystem::path metadataDirectory(const std::filesystem::path &root);

    struct WriteResult
    {
        bool ok;
        std::string message;
    };

    WriteResult writeTextFile(const std::filesystem::path &file,
                              const std::string &text);

    struct ReadResult
    {
        bool ok;
        std::string text;
        std::string message;
    };

    ReadResult readTextFile(const std::filesystem::path &file);

} // namespace minigit
