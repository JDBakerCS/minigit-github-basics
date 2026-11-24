#include "FileSystem.hpp"
#include "IndexTable.hpp"
#include "MiniGitRepository.hpp"
#include "RepositoryPaths.hpp"
#include "SnapshotStore.hpp"
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: missing command. Usage: minigit init\n";
        return 1;
    }

    const std::string command = argv[1];
    if (command != "init") {
        std::cerr << "Error: unsupported command '" << command << "'. Usage: minigit init\n";
        return 1;
    }

    if (argc > 2) {
        std::cerr << "Error: too many arguments for 'init'. Usage: minigit init\n";
        return 1;
    }

    try {
        minigit::RepositoryPaths paths(std::filesystem::current_path());
        minigit::FileSystem fileSystem;
        minigit::IndexTable index(paths, fileSystem);
        minigit::SnapshotStore snapshots(paths, fileSystem);
        minigit::MiniGitRepository repository(paths, fileSystem, index, snapshots);

        return repository.init();
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}