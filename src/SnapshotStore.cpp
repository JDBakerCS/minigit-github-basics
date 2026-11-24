#include "SnapshotStore.hpp"

namespace minigit {

SnapshotStore::SnapshotStore(const RepositoryPaths& paths, FileSystem& fileSystem)
    : headFile_(paths.headFile()),
      commitsRoot_(paths.metadataRoot() / "commits"),
      fileSystem_(fileSystem) {}

} // namespace minigit
