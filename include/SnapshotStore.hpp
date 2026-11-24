#pragma once
#include "FileSystem.hpp"
#include "RepositoryPaths.hpp"
#include "SnapshotRecord.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace minigit {

// Saves and validates snapshots and HEAD, and walks the history.
class SnapshotStore {
public:
    // Only stores locations and the file system; reads nothing.
    SnapshotStore(const RepositoryPaths& paths, FileSystem& fileSystem);  // M2

    std::optional<std::string> loadHeadId() const;                // M3
    std::optional<SnapshotRecord> loadHead() const;               // M3
    SnapshotRecord loadSnapshot(const std::string& snapshotId) const;  // M3
    std::string createId() const;                                 // M4
    void saveSnapshot(const SnapshotRecord& record);              // M4
    void updateHead(const std::string& snapshotId);               // M4
    std::vector<SnapshotRecord> collectHistory() const;           // M5

private:
    std::filesystem::path headFile_;      // .minigit/HEAD
    std::filesystem::path commitsRoot_;   // .minigit/commits
    FileSystem& fileSystem_;              // file access
};

}