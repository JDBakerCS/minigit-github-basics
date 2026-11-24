#pragma once
#include "FileSystem.hpp"
#include <map>
#include <optional>
#include <string>

namespace minigit {

// Each normalized project-relative path and its exact bytes. Used from M3.
using Snapshot = std::map<std::string, FileBytes>;

// One saved snapshot (commit). Used from M3.
struct SnapshotRecord {
    std::string id;                          // the snapshot's ID
    std::string message;                     // commit message, exactly as typed
    std::optional<std::string> previousId;   // empty for the first snapshot
    Snapshot files;                          // every recorded file and its bytes
};

}