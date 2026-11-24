#pragma once
#include "FileSystem.hpp"
#include "IndexTable.hpp"
#include "RepositoryPaths.hpp"
#include "SnapshotRecord.hpp"
#include "SnapshotStore.hpp"
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace minigit {

// More than one can apply to a path. Used from M3.
enum class StatusState {
    Untracked,
    Staged,
    ChangedAfterStaging,
    Modified,
    Deleted,
    Clean
};

// Each path and every state that applies to it. Used from M3.
using StatusReport = std::map<std::string, std::vector<StatusState>>;

// Enforces command rules, builds snapshots, coordinates saving/HEAD/cleanup, and prints results.
class MiniGitRepository {
public:
    // Only stores references; creates and reads nothing. The helpers must outlive this object.
    MiniGitRepository(RepositoryPaths& paths, FileSystem& fileSystem,
                      IndexTable& index, SnapshotStore& snapshots);   // M2

    int init();                                                   // M2, #35
    int addFile(const std::string& argument);                     // M3
    int status() const;                                           // M3
    StatusReport classifyStatus(const Snapshot& working,
                                const Snapshot& selected,
                                const std::optional<SnapshotRecord>& head) const;  // M3
    int commit(const std::string& message);                       // M4
    bool hasSelectedChanges(const Snapshot& selected,
                            const std::optional<SnapshotRecord>& head) const;     // M4
    SnapshotRecord buildSnapshot(const std::string& message,
                                 const Snapshot& selected,
                                 const std::optional<SnapshotRecord>& head) const; // M4
    int log() const;                                              // M5

private:
    RepositoryPaths& paths_;        // locations
    FileSystem& fileSystem_;        // file access
    IndexTable& index_;             // selections
    SnapshotStore& snapshots_;      // snapshots and HEAD
};

}