#include "RepositoryPaths.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>

namespace {

namespace fs = std::filesystem;

// Automatically remove the temporary folder when this object
// goes out of scope, including during exception handling.
struct TemporaryProject {
    fs::path root;

    TemporaryProject() {
        const auto timestamp =
            std::chrono::steady_clock::now()
                .time_since_epoch()
                .count();

        for (int attempt = 0; attempt < 100; ++attempt) {
            const fs::path candidate =
                fs::temp_directory_path() /
                ("minigit-rp6-" + std::to_string(timestamp) +
                 "-" + std::to_string(attempt));

            if (fs::create_directory(candidate)) {
                root = candidate;
                return;
            }
        }

        throw std::runtime_error(
            "Unable to create a unique temporary project.");
    }

    ~TemporaryProject() {
        std::error_code error;
        fs::remove_all(root, error);

        if (error) {
            std::cerr << "Temporary folder cleanup failed: "
                      << error.message() << '\n';
        }
    }

    TemporaryProject(const TemporaryProject&) = delete;
    TemporaryProject& operator=(const TemporaryProject&) = delete;
};

void requireRejected(
    const minigit::RepositoryPaths& paths,
    const std::string& input) {

    try {
        paths.safeRelativeFile(input);
    } catch (const std::runtime_error&) {
        return;
    }

    throw std::runtime_error(
        "Unsafe path was accepted: " + input);
}

void testRoots() {
    const fs::path roots[] = {
        fs::path("course") / "repo",
        fs::path("course") / "team" / "repo"
    };

    for (const auto& root : roots) {
        const minigit::RepositoryPaths paths(root);

        if (paths.projectRoot() != root) {
            throw std::runtime_error(
                "Project root check failed.");
        }

        if (paths.metadataRoot() != root / ".minigit") {
            throw std::runtime_error(
                "Metadata root check failed.");
        }
    }
}

void testRelativePaths() {
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
        const std::string result =
            paths.safeRelativeFile(test.input);

        if (result != test.expected) {
            throw std::runtime_error(
                "Wrong normalized result for: " + test.input);
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
        requireRejected(paths, input);
    }
}

void testSymbolicLinks() {
    TemporaryProject temporary;
    const minigit::RepositoryPaths paths(temporary.root);

    fs::create_directory(temporary.root / "docs");

    // Create a real file.
    {
        std::ofstream file(temporary.root / "docs" / "notes.txt");
        file << "RP-6 test file\n";
        file.close();

        if (!file) {
            throw std::runtime_error(
                "Unable to write the test file.");
        }
    }

    // A real file must still be accepted.
    if (paths.safeRelativeFile("docs/notes.txt") !=
        "docs/notes.txt") {
        throw std::runtime_error(
            "Real file path check failed.");
    }

    // A nonexistent path must still be accepted.
    if (paths.safeRelativeFile("missing/notes.txt") !=
        "missing/notes.txt") {
        throw std::runtime_error(
            "Missing file path check failed.");
    }

    std::error_code error;

    // Create a symbolic link to the real file.
    fs::create_symlink(
        temporary.root / "docs" / "notes.txt",
        temporary.root / "file-link.txt",
        error);

    if (error) {
        std::cout << "File symbolic-link test skipped: "
                  << error.message() << '\n';
    } else {
        requireRejected(paths, "file-link.txt");
    }

    error.clear();

    // Create a symbolic link to the docs folder.
    fs::create_directory_symlink(
        temporary.root / "docs",
        temporary.root / "folder-link",
        error);

    if (error) {
        std::cout << "Folder symbolic-link test skipped: "
                  << error.message() << '\n';
    } else {
        requireRejected(paths, "folder-link/notes.txt");
    }

    // Broken symbolic links must also be rejected.
    error.clear();

    fs::create_symlink(
        temporary.root / "does-not-exist.txt",
        temporary.root / "broken-link.txt",
        error);

    if (error) {
        std::cout << "Broken symbolic-link test skipped: "
                  << error.message() << '\n';
    } else {
        requireRejected(paths, "broken-link.txt");
    }
}

} // namespace

int main() {
    try {
        testRoots();
        testRelativePaths();
        testSymbolicLinks();

        std::cout << "RepositoryPaths checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "RepositoryPaths test failed: "
                  << error.what() << '\n';
        return 1;
    }
}