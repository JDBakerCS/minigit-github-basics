#include "MiniGitRepository.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace minigit {

MiniGitRepository::MiniGitRepository(
    RepositoryPaths& paths, FileSystem& fileSystem,
    IndexTable& index, SnapshotStore& snapshots)
    : paths_(paths),
      fileSystem_(fileSystem),
      index_(index),
      snapshots_(snapshots) {}

int MiniGitRepository::init() {
    const auto metadata = paths_.metadataRoot();

    try {
        // An existing .minigit must be valid. Never repair or overwrite it.
        if (std::filesystem::exists(metadata) ||
            std::filesystem::is_symlink(metadata)) {
            if (paths_.isInitialized()) {
                std::cout << "Already a MiniGit repository\n";
                return 0;
            }
            std::cerr << "Error: MiniGit metadata is incomplete or invalid\n";
            return 1;
        }

        try {
            fileSystem_.createDirectories(metadata / "stage");
            fileSystem_.createDirectories(metadata / "commits");
            fileSystem_.writeBytesSafely(paths_.headFile(), {});
            fileSystem_.writeBytesSafely(paths_.indexFile(), {});
        } catch (...) {
            // This command started with no .minigit, so only its new metadata
            // can be removed. Working files are never touched.
            std::error_code cleanupError;
            std::filesystem::remove_all(metadata, cleanupError);
            throw;
        }

        std::cout << "Initialized MiniGit repository\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}

} // namespace minigit
