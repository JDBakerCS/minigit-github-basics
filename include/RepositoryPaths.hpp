#pragma once

#include <filesystem>
#include <string>

namespace minigit {

class RepositoryPaths {
public:
    explicit RepositoryPaths(std::filesystem::path projectRoot);

    const std::filesystem::path& projectRoot() const;
    const std::filesystem::path& metadataRoot() const;

    std::string safeRelativeFile(const std::string& argument) const;

private:
    std::filesystem::path projectRoot_;
    std::filesystem::path metadataRoot_;
};

} // namespace minigit