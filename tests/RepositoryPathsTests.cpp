#include "RepositoryPaths.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

int main() {
    // Existing RP-1 tests.
    const std::filesystem::path roots[] = {
        std::filesystem::path("course") / "repo",
        std::filesystem::path("course") / "team" / "repo"
    };

    for (const auto& root : roots) {
        const minigit::RepositoryPaths paths(root);

        if (paths.projectRoot() != root) {
            std::cerr << "Project root check failed\n";
            return 1;
        }

        if (paths.metadataRoot() != root / ".minigit") {
            std::cerr << "Metadata root check failed\n";
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

    // RP-5 tests. This directory does not need to exist.
    const minigit::RepositoryPaths paths("course/repo");

    struct AcceptedCase {
        std::string input;
        std::string expected;
    };

    const AcceptedCase accepted[] = {
        {"notes.txt", "notes.txt"},
        {"docs/notes.txt", "docs/notes.txt"},
        {"docs/./notes.txt", "docs/notes.txt"},
        {"docs/../notes.txt", "notes.txt"},
        {"my..notes.txt", "my..notes.txt"}
    };

    for (const auto& test : accepted) {
        try {
            const std::string result =
                paths.safeRelativeFile(test.input);

            if (result != test.expected) {
                std::cerr << "Wrong result for: " << test.input
                          << "\nExpected: " << test.expected
                          << "\nReceived: " << result << '\n';
                return 1;
            }
        } catch (const std::exception& error) {
            std::cerr << "Valid path was rejected: " << test.input
                      << "\nError: " << error.what() << '\n';
            return 1;
        }
    }

    const std::string rejected[] = {
        "",
        ".",
        "/notes.txt",
        "../notes.txt",
        "docs/../../notes.txt",
        ".minigit/HEAD",
        ".git/config",
        "docs/.minigit/x"
    };

    for (const auto& input : rejected) {
        try {
            paths.safeRelativeFile(input);
        } catch (const std::runtime_error&) {
            // Expected: the unsafe path was rejected.
            continue;
        } catch (const std::exception& error) {
            std::cerr << "Wrong exception for: " << input
                      << "\nError: " << error.what() << '\n';
            return 1;
        }

        std::cerr << "Unsafe path was accepted: " << input << '\n';
        return 1;
    }

    std::cout << "RepositoryPaths checks passed\n";
    return 0;
}
