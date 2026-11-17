#include "RepositoryPaths.hpp"

#include <stdexcept>
#include <system_error>

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

    // RP-5: Reject an empty argument.
    if (argument.empty()) {
        throw std::runtime_error("File path must not be empty.");
    }

    const std::filesystem::path path(argument);

    // RP-5: Reject absolute or rooted paths.
    if (path.has_root_path()) {
        throw std::runtime_error("File path must be relative.");
    }

    // RP-5: Normalize the path.
    const std::filesystem::path clean = path.lexically_normal();

    if (clean.empty() || clean == ".") {
        throw std::runtime_error("File path must name a file.");
    }

    // RP-5: Reject paths that leave the project.
    if (*clean.begin() == "..") {
        throw std::runtime_error(
            "File path must stay inside the project.");
    }

    // RP-5: Reject Git and MiniGit metadata paths.
    for (const auto& part : clean) {
        if (part == ".git" || part == ".minigit") {
            throw std::runtime_error(
                "File path must not access Git or MiniGit metadata.");
        }
    }

    // RP-6: Walk through each part without following symbolic links.
    std::filesystem::path current = projectRoot_;

    for (const auto& part : clean) {
        current /= part;

        std::error_code error;
        const auto status =
            std::filesystem::symlink_status(current, error);

        // Missing paths are still allowed.
        if (error == std::errc::no_such_file_or_directory) {
            continue;
        }

        if (error) {
            throw std::runtime_error(
                "Unable to inspect file path: " + error.message());
        }

        if (std::filesystem::is_symlink(status)) {
            throw std::runtime_error(
                "File path must not contain symbolic links.");
        }
    }

    // Store paths using forward slashes on every platform.
    return clean.generic_string();
}

std::filesystem::path RepositoryPaths::commitDirectory(const std::string &id) const {
    return metadataRoot_ / "commits" / id;
}

std::filesystem::path RepositoryPaths::committedFile(const std::string &id,
    const std::string &path) const {
    return commitDirectory(id) / "files" / path;
}
std::filesystem::path RepositoryPaths::headFile() const {
    return metadataRoot_ / "HEAD";
}

std::filesystem::path RepositoryPaths::indexFile() const {
    return metadataRoot_ / "index.txt";
}
std::filesystem::path RepositoryPaths::stageFile(const std::string &path) const {
    return metadataRoot_ / "stage" / path;
}

} // namespace minigit
