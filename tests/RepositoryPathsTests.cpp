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