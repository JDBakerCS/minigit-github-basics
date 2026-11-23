#pragma once
#include "FileSystem.hpp"
#include "RepositoryPaths.hpp"
#include "SnapshotRecord.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace minigit {

class SnapshotStore {
public:

    SnapshotStore(const RepositoryPaths& paths, FileSystem& fileSystem);

    std::optional<std::string> loadHeadId() const;              
    std::optional<SnapshotRecord> loadHead() const;               
    SnapshotRecord loadSnapshot(const std::string& snapshotId) const;  
    std::string createId() const;                             
    void saveSnapshot(const SnapshotRecord& record);              
    void updateHead(const std::string& snapshotId);             
    std::vector<SnapshotRecord> collectHistory() const;           

private:
    std::filesystem::path headFile_;     
    std::filesystem::path commitsRoot_;   
    FileSystem& fileSystem_;              
};

}