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

enum class StatusState {
    Untracked,
    Staged,
    ChangedAfterStaging,
    Modified,
    Deleted,
    Clean
};

using StatusReport = std::map<std::string, std::vector<StatusState>>;

class MiniGitRepository {
public:
    MiniGitRepository(RepositoryPaths& paths, FileSystem& fileSystem,
                      IndexTable& index, SnapshotStore& snapshots);

    int init();                                                 
    int addFile(const std::string& argument);                   
    int status() const;                                    
    StatusReport classifyStatus(const Snapshot& working,
                                const Snapshot& selected,
                                const std::optional<SnapshotRecord>& head) const;  
    int commit(const std::string& message);                       
    bool hasSelectedChanges(const Snapshot& selected,
                            const std::optional<SnapshotRecord>& head) const;    
    SnapshotRecord buildSnapshot(const std::string& message,
                                 const Snapshot& selected,
                                 const std::optional<SnapshotRecord>& head) const; 
    int log() const;                                              

private:
    RepositoryPaths& paths_;       
    FileSystem& fileSystem_;        
    IndexTable& index_;             
    SnapshotStore& snapshots_;     
};

}