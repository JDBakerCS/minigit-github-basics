#include "MiniGitRepository.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

namespace fs = std::filesystem;

struct TemporaryProject {
    fs::path root;

    TemporaryProject() {
        const auto stamp = std::chrono::steady_clock::now()
                               .time_since_epoch().count();
        for (int attempt = 0; attempt < 100; ++attempt) {
            const auto candidate = fs::temp_directory_path() /
                ("minigit-init-" + std::to_string(stamp) + "-" +
                 std::to_string(attempt));
            if (fs::create_directory(candidate)) {
                root = candidate;
                return;
            }
        }
        throw std::runtime_error("Could not create a temporary project.");
    }

    ~TemporaryProject() {
        std::error_code ignored;
        fs::remove_all(root, ignored);
    }
};

struct Result {
    int code;
    std::string output;
    std::string error;
};

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void writeText(const fs::path& path, const std::string& text) {
    std::ofstream output(path, std::ios::binary);
    output << text;
    output.close();
    require(static_cast<bool>(output), "Could not write test file.");
}

std::string readText(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    require(static_cast<bool>(input), "Could not read test file.");
    return std::string(std::istreambuf_iterator<char>(input),
                       std::istreambuf_iterator<char>());
}

Result runInit(minigit::MiniGitRepository& repository) {
    std::ostringstream output;
    std::ostringstream error;
    auto* oldOutput = std::cout.rdbuf(output.rdbuf());
    auto* oldError = std::cerr.rdbuf(error.rdbuf());
    const int code = repository.init();
    std::cout.rdbuf(oldOutput);
    std::cerr.rdbuf(oldError);
    return {code, output.str(), error.str()};
}

struct TestRepository {
    minigit::RepositoryPaths paths;
    minigit::FileSystem files;
    minigit::IndexTable index;
    minigit::SnapshotStore snapshots;
    minigit::MiniGitRepository repository;

    explicit TestRepository(const fs::path& root)
        : paths(root),
          index(paths, files),
          snapshots(paths, files),
          repository(paths, files, index, snapshots) {}
};

void testNewRepository() {
    TemporaryProject project;
    writeText(project.root / "notes.txt", "Keep these bytes.\n");
    TestRepository test(project.root);
    require(!fs::exists(test.paths.metadataRoot()),
            "Constructors created metadata.");

    const Result result = runInit(test.repository);
    require(result.code == 0 && result.error.empty(),
            "Initial init failed.");
    require(test.paths.isInitialized(), "New metadata is invalid.");
    require(readText(project.root / "notes.txt") == "Keep these bytes.\n",
            "Init changed a Working file.");
    require(fs::is_empty(test.paths.headFile()) &&
            fs::is_empty(test.paths.indexFile()) &&
            fs::is_empty(test.paths.metadataRoot() / "stage") &&
            fs::is_empty(test.paths.metadataRoot() / "commits"),
            "New metadata is not empty.");

    std::set<std::string> entries;
    for (const auto& entry : fs::directory_iterator(test.paths.metadataRoot())) {
        entries.insert(entry.path().filename().string());
    }
    require(entries == std::set<std::string>{"stage", "commits", "HEAD", "index.txt"},
            "Init created unexpected metadata entries.");
}

void testRepeatedInit() {
    TemporaryProject project;
    TestRepository test(project.root);
    require(runInit(test.repository).code == 0, "First init failed.");

    writeText(test.paths.headFile(), "7");
    writeText(test.paths.indexFile(), "notes.txt\n");
    writeText(test.paths.stageFile("notes.txt"), "STAGED");
    writeText(project.root / "notes.txt", "WORKING");
    fs::create_directory(test.paths.commitDirectory("7"));
    writeText(test.paths.commitDirectory("7") / "message.txt", "Saved");

    const Result result = runInit(test.repository);
    require(result.code == 0 &&
            result.output.find("Already") != std::string::npos &&
            result.error.empty(), "Repeated init did not report a no-op.");
    require(readText(test.paths.headFile()) == "7" &&
            readText(test.paths.indexFile()) == "notes.txt\n" &&
            readText(test.paths.stageFile("notes.txt")) == "STAGED" &&
            readText(test.paths.commitDirectory("7") / "message.txt") == "Saved" &&
            readText(project.root / "notes.txt") == "WORKING",
            "Repeated init changed existing data.");
}

void testDamagedMetadata() {
    TemporaryProject project;
    TestRepository test(project.root);
    require(runInit(test.repository).code == 0, "Setup init failed.");
    writeText(test.paths.indexFile(), "keep");
    fs::remove(test.paths.headFile());

    const Result result = runInit(test.repository);
    require(result.code == 1 &&
            result.error.find("Error:") != std::string::npos,
            "Damaged metadata was not rejected.");
    require(!fs::exists(test.paths.headFile()) &&
            readText(test.paths.indexFile()) == "keep",
            "Damaged metadata was repaired or changed.");
}

void testConflictingMetadata() {
    TemporaryProject project;
    writeText(project.root / ".minigit", "keep");
    TestRepository test(project.root);

    const Result result = runInit(test.repository);
    require(result.code == 1 &&
            result.error.find("Error:") != std::string::npos &&
            readText(project.root / ".minigit") == "keep",
            "Conflicting metadata was overwritten.");
}

void testFailedCreation() {
    TemporaryProject project;
    writeText(project.root / "blocker", "keep");
    TestRepository test(project.root / "blocker");

    const Result result = runInit(test.repository);
    require(result.code == 1 &&
            result.error.find("Error:") != std::string::npos &&
            readText(project.root / "blocker") == "keep",
            "Failed init changed a pre-existing file.");
}

} // namespace

int main() {
    try {
        testNewRepository();
        testRepeatedInit();
        testDamagedMetadata();
        testConflictingMetadata();
        testFailedCreation();
        std::cout << "Initialization checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Initialization test failed: " << error.what() << '\n';
        return 1;
    }
}
