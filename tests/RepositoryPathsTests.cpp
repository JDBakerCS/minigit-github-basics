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
const std::filesystem::path stageRoot = root / ".minigit" / "stage";

if (paths.stageFile("notes.txt") != stageRoot / "notes.txt") {
std::cerr << "stageFile top-level check failed: got "
<< paths.stageFile("notes.txt").string() << "\n";
return 1;
    }

if (paths.stageFile("docs/notes.txt") != stageRoot / "docs" / "notes.txt") {
std::cerr << "stageFile sub-folder check failed: got "
<< paths.stageFile("docs/notes.txt").string() << "\n";
return 1;
    }

if (paths.stageFile("docs/notes.txt").filename() != "notes.txt") {
std::cerr << "stageFile filename check failed\n";
return 1;
    }

if (std::filesystem::exists(paths.metadataRoot()) ||
std::filesystem::exists(stageRoot)) {
std::cerr << "files were created on disk\n";
return 1;
    }
  }
    
  

  std::cout << "RepositoryPaths root checks passed\n";
  return 0;
}