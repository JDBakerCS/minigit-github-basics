#pragma once
#include "FileSystem.hpp"
#include "RepositoryPaths.hpp"
#include "SnapshotRecord.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace minigit {

// Reads, updates, and clears the selected paths (index.txt) and their Stage copies.
class IndexTable {
public:
    // Only stores locations and the file system; reads nothing.
    IndexTable(const RepositoryPaths& paths, FileSystem& fileSystem);  // M2

    std::vector<std::string> loadPaths() const;                   // M3
    bool contains(const std::string& relativePath) const;         // M3
    void savePaths(const std::vector<std::string>& entries);      // M3
    Snapshot loadSelections() const;                              // M3
    void clearSelections();                                       // M4

private:
    std::filesystem::path indexFile_;   // .minigit/index.txt
    std::filesystem::path stageRoot_;   // .minigit/stage
    FileSystem& fileSystem_;            // file access
};

}