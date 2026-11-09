#include "RepositoryPaths.hpp"

#include <stdexcept>

namespace minigit {

RepositoryPaths::RepositoryPaths(std::filesystem::path projectRoot)
    : projectRoot_(projectRoot),
      metadataRoot_(projectRoot_ / ".minigit") {}

const std::filesystem::path& RepositoryPaths::projectRoot() const {
    return projectRoot_;
}

const std::filesystem::path& RepositoryPaths::metadataRoot() const {
    return metadataRoot_;
}

std::string RepositoryPaths::safeRelativeFile(
    const std::string& argument) const {

    // Issue: "Rejects an empty argument."
    if (argument.empty()) {
        throw std::runtime_error("File path must not be empty.");
    }

    const std::filesystem::path path(argument);

    // Issue: "Rejects absolute or rooted paths using has_root_path()."
    if (path.has_root_path()) {
        throw std::runtime_error("File path must be relative.");
    }

    // Issue: "Normalises with lexically_normal()."
    const std::filesystem::path clean = path.lexically_normal();

    // Issue: "Rejects a result that is . or empty."
    if (clean.empty() || clean == ".") {
        throw std::runtime_error("File path must name a file.");
    }

    // Issue: After normalising, reject if the first part is "..".
    if (*clean.begin() == "..") {
        throw std::runtime_error("File path must stay inside the project.");
    }

    // Issue: Reject any part equal to ".git" or ".minigit".
    for (const auto& part : clean) {
        if (part == ".git" || part == ".minigit") {
            throw std::runtime_error(
                "File path must not access Git or MiniGit metadata.");
        }
    }

    // Issue: Use "/" separators on every platform.
    return clean.generic_string();
}

} // namespace minigit