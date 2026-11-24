#include "IndexTable.hpp"

namespace minigit {

IndexTable::IndexTable(const RepositoryPaths& paths, FileSystem& fileSystem)
    : indexFile_(paths.indexFile()),
      stageRoot_(paths.metadataRoot() / "stage"),
      fileSystem_(fileSystem) {}

} // namespace minigit
