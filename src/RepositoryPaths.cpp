#include "RepositoryPaths.hpp"

namespace minigit {

RepositoryPaths::RepositoryPaths(std::filesystem::path projectRoot)
    : projectRoot_(projectRoot), metadataRoot_(projectRoot_ / ".minigit") {}

const std::filesystem::path &RepositoryPaths::projectRoot() const {
    return projectRoot_;
}
const std::filesystem::path &RepositoryPaths::metadataRoot() const {
    return metadataRoot_;
}
std::filesystem::path RepositoryPaths::headFile() const {
    return metadataRoot_ / "HEAD";
}

std::filesystem::path RepositoryPaths::indexFile() const {
    return metadataRoot_ / "index.txt";
}
} // namespace minigit