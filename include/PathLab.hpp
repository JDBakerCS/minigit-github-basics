#pragma once
#include <filesystem>

namespace minigit {

// Week 1 Issues will add small helpers for building paths below a repository
// root. Path construction describes locations; it does not create files or
// directories.
std::filesystem::path metadataDirectory(const std::filesystem::path &root);
} // namespace minigit
