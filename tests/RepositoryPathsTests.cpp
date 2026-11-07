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

const std::filesystem::path commitRoot = root / ".minigit" / "commits" / "3";

if (paths.commitDirectory("3") != commitRoot) {
std::cerr << "commitDirectory check failed: got "
<< paths.commitDirectory("3").string() << "\n";
return 1;
    }

if (paths.committedFile("3", "docs/notes.txt") !=
commitRoot / "files" / "docs" / "notes.txt") {
std::cerr << "committedFile check failed: got "
<< paths.committedFile("3", "docs/notes.txt").string() << "\n";
return 1;
    }

if (paths.committedFile("3", "a.txt").parent_path() !=
paths.commitDirectory("3") / "files") {
std::cerr << "committedFile parent check failed\n";
return 1;
    }

if (std::filesystem::exists(paths.metadataRoot()) ||
std::filesystem::exists(commitRoot)) {
std::cerr << "files were created on disk\n";
return 1;
    }
  }
  std::cout << "RepositoryPaths root checks passed\n";
  return 0;
}