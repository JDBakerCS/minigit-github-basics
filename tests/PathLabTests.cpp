#include "PathLab.hpp"
#include <iostream>

int main() {
    // Baseline smoke check: the project and CTest wiring work before students
    // begin the path Issues. Add focused assertions with each Issue.
    std::filesystem::path sample = "course/repo";
    if (sample.filename() != "repo") {
        std::cerr << "path smoke check failed\n";
        return 1;
    }

    const std::filesystem::path ordinaryRoot = "repo";
    const std::filesystem::path nestedRoot =
        std::filesystem::path("course") / "repo";

    if (minigit::metadataDirectory(ordinaryRoot) !=
        ordinaryRoot / ".minigit") {
        std::cerr << "metadata directory path failed for ordinary root\n";
        return 1;
    }
    if (minigit::metadataDirectory(nestedRoot) !=
        nestedRoot / ".minigit") {
        std::cerr << "metadata directory path failed for nested root\n";
        return 1;
    }

    if (minigit::headFilePath(ordinaryRoot) !=
        ordinaryRoot / ".minigit" / "HEAD") {
        std::cerr << "HEAD file path failed for ordinary root\n";
        return 1;
    }
    if (minigit::headFilePath(nestedRoot) !=
        nestedRoot / ".minigit" / "HEAD") {
        std::cerr << "HEAD file path failed for nested root\n";
        return 1;
    }

    std::cout << "PathLab path checks passed\n";
    return 0;
}
