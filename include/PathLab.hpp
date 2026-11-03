#pragma once
#include <filesystem>

namespace minigit {

std::filesystem::path headFilePath(const std::filesystem::path &root);

std::filesystem::path metadataDirectory(const std::filesystem::path &root);
} // namespace minigit
