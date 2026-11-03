#pragma once
#include <filesystem>

namespace minigit {

std::filesystem::path headFilePath(const std::filesystem::path &root);

std::filesystem::path metadataDirectory(const std::filesystem::path &root);

struct WriteResult {
    bool ok;
    std::string message;
};

WriteResult writeTextFile(const std::filesystem::path &file,
                          const std::string &text);
} // namespace minigit
