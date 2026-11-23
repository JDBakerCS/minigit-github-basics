#pragma once
#include "FileSystem.hpp"
#include <map>
#include <optional>
#include <string>

namespace minigit {


using Snapshot = std::map<std::string, FileBytes>;


struct SnapshotRecord {
    std::string id;                        
    std::string message;                     
    std::optional<std::string> previousId;   
    Snapshot files;                          
};

}