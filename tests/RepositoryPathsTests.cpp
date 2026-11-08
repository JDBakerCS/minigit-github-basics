#include "RepositoryPaths.hpp"
#include <iostream>

int main() {
  const std::filesystem::path roots[] = {
      std::filesystem::path("course") / "repo",
      std::filesystem::path("course") / "team" / "repo"};

  for (const auto &root : roots) {
    const minigit::RepositoryPaths paths(root);

    if (paths.projectRoot() != root) {
      std::cerr << "project root check failed\n";
      return 1;
    }

    if (paths.metadataRoot() != root / ".minigit") {
      std::cerr << "metadata root check failed\n";
      return 1;
    }
    if (paths.headFile() != root / ".minigit" / "HEAD") {
      std::cerr << "HEAD path check failed\n";
      return 1;
    }

    if (paths.indexFile() != root / ".minigit" / "index.txt") {
      std::cerr << "index path check failed\n";
      return 1;
    }
    if (paths.headFile().parent_path() != paths.metadataRoot()) {
      std::cerr << "HEAD parent check failed\n";
      return 1;
    }

    if (paths.indexFile().filename() != "index.txt") {
      std::cerr << "index filename check failed\n";
      return 1;
    }
  }

  std::cout << "RepositoryPaths root checks passed\n";
  return 0;
}