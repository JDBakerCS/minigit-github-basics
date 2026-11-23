#pragma once
#include "FileSystem.hpp"
#include "RepositoryPaths.hpp"
#include "SnapshotRecord.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace minigit {


class IndexTable {
public:
    IndexTable(const RepositoryPaths& paths, FileSystem& fileSystem);

    std::vector<std::string> loadPaths() const;                   
    bool contains(const std::string& relativePath) const;         
    void savePaths(const std::vector<std::string>& entries);      
    Snapshot loadSelections() const;                             
    void clearSelections();                                       

private:
    std::filesystem::path indexFile_;   
    std::filesystem::path stageRoot_;   
    FileSystem& fileSystem_;           
};

}