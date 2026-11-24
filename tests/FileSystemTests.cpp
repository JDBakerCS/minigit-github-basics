#include "FileSystem.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

// Report a failed check without stopping the rest of the checks in that test.
bool check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }
    return true;
}

bool testRemove() {
    const minigit::FileSystem fileSystem;
    const auto temp = std::filesystem::temp_directory_path() / "fs4-tests";
    std::filesystem::remove_all(temp);
    std::filesystem::create_directories(temp / "docs");
    std::ofstream(temp / "a.txt") << "a";
    std::ofstream(temp / "b.txt") << "b";
    std::ofstream(temp / "docs" / "notes.txt") << "notes";

    bool passed = true;

    // Deletes a.txt and leaves b.txt alone.
    fileSystem.remove(temp / "a.txt");
    passed = check(!std::filesystem::exists(temp / "a.txt"), "a.txt is deleted") && passed;
    passed = check(std::filesystem::exists(temp / "b.txt"), "b.txt is kept") && passed;

    // Deletes a file in a folder, but keeps the folder.
    fileSystem.remove(temp / "docs" / "notes.txt");
    passed = check(!std::filesystem::exists(temp / "docs" / "notes.txt"), "docs/notes.txt is deleted") && passed;
    passed = check(std::filesystem::exists(temp / "docs"), "docs folder is kept") && passed;

    // Error case: a missing file throws.
    bool threw = false;
    std::string message;
    try {
        fileSystem.remove(temp / "missing.txt");
    } catch (const std::runtime_error& error) {
        threw = true;
        message = error.what();
    }
    passed = check(threw, "missing file throws") && passed;
    passed = check(message.find("missing.txt") != std::string::npos, "message names the file") && passed;

    std::filesystem::remove_all(temp);
    return passed;
}

bool testReadBytes() {
    const minigit::FileSystem fileSystem;
    // The clock value helps separate this run's temporary folder from other runs.
    const auto temp = std::filesystem::temp_directory_path() /
        ("fs1-tests-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    if (!std::filesystem::create_directory(temp)) {
        throw std::runtime_error("Cannot create test directory");
    }

    bool passed = true;
    try {
        const minigit::FileBytes binary = {0, 10, 13, 26, 127, 128, 255};
        // This scope closes the output files before readBytes() opens them.
        {
            std::ofstream text(temp / "text.txt", std::ios::binary);
            text << "hello\r\n";
            std::ofstream output(temp / "binary.bin", std::ios::binary);
            output.write(reinterpret_cast<const char*>(binary.data()), binary.size());
            std::ofstream empty(temp / "empty.bin", std::ios::binary);
            if (!text || !output || !empty) {
                throw std::runtime_error("Cannot create test files");
            }
        }

        passed = check(fileSystem.isRegularFile(temp / "text.txt"), "regular file") && passed;
        passed = check(!fileSystem.isRegularFile(temp), "directory is not a regular file") && passed;
        passed = check(!fileSystem.isRegularFile(temp / "missing.txt"), "missing file returns false") && passed;
        const minigit::FileBytes text = {'h', 'e', 'l', 'l', 'o', '\r', '\n'};
        passed = check(fileSystem.readBytes(temp / "text.txt") == text, "text bytes preserved") && passed;
        passed = check(fileSystem.readBytes(temp / "binary.bin") == binary, "binary bytes preserved") && passed;
        passed = check(fileSystem.readBytes(temp / "empty.bin").empty(), "empty file returns empty bytes") && passed;

        for (const auto& path : {temp / "missing.txt", temp}) {
            bool threw = false;
            try {
                fileSystem.readBytes(path);
            } catch (const std::runtime_error&) {
                threw = true;
            }
            passed = check(threw, "missing file or directory throws") && passed;
        }
        passed = check(!std::filesystem::exists(temp / "missing.txt"), "missing file stays missing") && passed;
        passed = check(std::filesystem::is_directory(temp), "directory is unchanged") && passed;
        passed = check(fileSystem.readBytes(temp / "binary.bin") == binary, "file remains unchanged") && passed;
    } catch (...) {
        std::filesystem::remove_all(temp);
        throw;
    }
    std::filesystem::remove_all(temp);
    return passed;
}

// Check creating a file, replacing it with fewer bytes, and making it empty.
bool testWriteBytesSafely() {
    const minigit::FileSystem fileSystem;
    const auto temp = std::filesystem::temp_directory_path() /
        ("fs2-tests-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    if (!std::filesystem::create_directory(temp)) {
        throw std::runtime_error("Cannot create test directory");
    }

    bool passed = true;
    try {
        const auto target = temp / "data.bin";
        // include a null byte, line endings, and values above 127 to test binary data
        const minigit::FileBytes binary = {0, 10, 13, 26, 128, 255};
        fileSystem.writeBytesSafely(target, binary);
        // keep an earlier failure in passed, while still running each new check
        passed = check(fileSystem.readBytes(target) == binary, "write preserves binary bytes") && passed;
        // A shorter replacement must remove all bytes from the old version
        fileSystem.writeBytesSafely(target, {42});
        passed = check(fileSystem.readBytes(target) == minigit::FileBytes{42}, "shorter replacement has no leftover bytes") && passed;
        // An empty byte vector should produce an existing file with zero bytes
        fileSystem.writeBytesSafely(target, {});
        passed = check(fileSystem.readBytes(target).empty(), "empty replacement") && passed;
        passed = check(!std::filesystem::exists(temp / "data.bin.tmp"), "successful writes clean temporary workspace") && passed;
    } catch (...) {
        std::filesystem::remove_all(temp);
        throw;
    }
    std::filesystem::remove_all(temp);
    return passed;
}

// check errors that happen before the original file can be replaced.
bool testWriteErrors() {
    const minigit::FileSystem fileSystem;
    const auto temp = std::filesystem::temp_directory_path() /
        ("fs2-errors-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    if (!std::filesystem::create_directory(temp)) {
        throw std::runtime_error("Cannot create test directory");
    }

    bool passed = true;
    try {
        const auto target = temp / "data.bin";
        const minigit::FileBytes binary = {0, 10, 13, 26, 128, 255};

        bool threw = false;
        try {
            fileSystem.writeBytesSafely(temp / "missing" / "data.bin", binary);
        } catch (const std::runtime_error&) {
            threw = true;
        }
        passed = check(threw, "missing parent throws") && passed;
        passed = check(!std::filesystem::exists(temp / "missing"), "missing parent is not created") && passed;

        // occupy the temporary workspace to simulate failure before replacement.
        // The function must leave this existing workspace and the original file alone
        fileSystem.writeBytesSafely(target, binary);
        std::filesystem::create_directory(temp / "data.bin.tmp");
        threw = false;
        try {
            fileSystem.writeBytesSafely(target, {42});
        } catch (const std::runtime_error&) {
            threw = true;
        }
        passed = check(threw, "occupied temporary workspace throws") && passed;
        passed = check(fileSystem.readBytes(target) == binary, "failure preserves original bytes") && passed;
        passed = check(std::filesystem::is_empty(temp / "data.bin.tmp"), "failure creates no temporary file") && passed;
        std::filesystem::remove(temp / "data.bin.tmp");
    } catch (...) {
        std::filesystem::remove_all(temp);
        throw;
    }
    std::filesystem::remove_all(temp);
    return passed;
}

// check cleanup when temporary bytes are written but replacement cannot finish
bool testFailedReplacement() {
    const minigit::FileSystem fileSystem;
    const auto temp = std::filesystem::temp_directory_path() /
        ("fs2-replacement-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    if (!std::filesystem::create_directory(temp)) {
        throw std::runtime_error("Cannot create test directory");
    }

    bool passed = true;
    try {
        const minigit::FileBytes binary = {0, 10, 13, 26, 128, 255};

        // A directory cannot be replaced by a file; staged bytes must be cleaned up
        const auto folder = temp / "folder";
        std::filesystem::create_directory(folder);
        fileSystem.writeBytesSafely(folder / "keep.bin", binary);
        bool threw = false;
        try {
            fileSystem.writeBytesSafely(folder, {42});
        } catch (const std::runtime_error&) {
            threw = true;
        }
        passed = check(threw, "replacing a directory throws") && passed;
        passed = check(fileSystem.readBytes(folder / "keep.bin") == binary, "failed replacement preserves existing contents") && passed;
        passed = check(!std::filesystem::exists(temp / "folder.tmp"), "failed replacement cleans temporary workspace") && passed;
    } catch (...) {
        // Clean up even after an unexpected exception, then report it to the caller.
        std::filesystem::remove_all(temp);
        throw;
    }
    // Normal cleanup also runs when a check returned false without throwing.
    std::filesystem::remove_all(temp);
    return passed;
}

bool testCopy() {
    const minigit::FileSystem fileSystem;
    const auto temp = std::filesystem::temp_directory_path() /
        ("fs3-tests-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    if (!std::filesystem::create_directory(temp)) {
        throw std::runtime_error("Cannot create test directory");
    }

    bool passed = true;
    try {
        const auto source = temp / "source.bin";
        const auto destination = temp / "copy.bin";
        const minigit::FileBytes original = {0, 10, 13, 26, 127, 128, 255};
        const minigit::FileBytes oldDestination = {'o', 'l', 'd'};

        // Copy a regular file to a new destination path and verify exact content match.
        fileSystem.writeBytesSafely(source, original);
        fileSystem.copy(source, destination);
        passed = check(fileSystem.readBytes(destination) == original, "copy preserves exact binary bytes") && passed;
        passed = check(fileSystem.readBytes(source) == original, "copy leaves source unchanged") && passed;

        // Overwrite an existing destination file and verify updated content.
        fileSystem.writeBytesSafely(destination, oldDestination);
        fileSystem.copy(source, destination);
        passed = check(fileSystem.readBytes(destination) == original, "copy overwrites existing destination") && passed;
        passed = check(fileSystem.readBytes(source) == original, "overwrite leaves source unchanged") && passed;

        // Error case: source does not exist or is a directory.
        for (const auto& invalidSource : {temp / "missing.bin", temp}) {
            bool threw = false;
            try {
                fileSystem.copy(invalidSource, temp / "out.bin");
            } catch (const std::runtime_error&) {
                threw = true;
            }
            passed = check(threw, "missing source or directory throws") && passed;
        }
        passed = check(!std::filesystem::exists(temp / "out.bin"), "invalid source creates no destination") && passed;

        // Error case: destination parent directory does not exist.
        bool threw = false;
        try {
            fileSystem.copy(source, temp / "missing-dir" / "copy.bin");
        } catch (const std::runtime_error&) {
            threw = true;
        }
        passed = check(threw, "missing destination parent throws") && passed;
        passed = check(!std::filesystem::exists(temp / "missing-dir"), "missing parent directory is not created") && passed;
    } catch (...) {
        std::filesystem::remove_all(temp);
        throw;
    }
    std::filesystem::remove_all(temp);
    return passed;
}

// Check creating nested folders, an existing folder, and files in the way.
bool testCreateDirectories() {
    const minigit::FileSystem fileSystem;
    const auto temp = std::filesystem::temp_directory_path() / "m2-2-tests";
    std::filesystem::remove_all(temp);
    std::filesystem::create_directories(temp);

    bool passed = true;

    // Creates a folder and its missing parent.
    fileSystem.createDirectories(temp / "a" / "b");
    passed = check(std::filesystem::is_directory(temp / "a"), "parent folder is created") && passed;
    passed = check(std::filesystem::is_directory(temp / "a" / "b"), "folder is created") && passed;

    // Calling it again on an existing folder does nothing.
    bool threw = false;
    try {
        fileSystem.createDirectories(temp / "a" / "b");
    } catch (const std::runtime_error&) {
        threw = true;
    }
    passed = check(!threw, "existing folder does not throw") && passed;

    // Error case: a regular file at the target path throws and stays unchanged.
    std::ofstream(temp / "file.txt") << "keep";
    threw = false;
    try {
        fileSystem.createDirectories(temp / "file.txt");
    } catch (const std::runtime_error&) {
        threw = true;
    }
    passed = check(threw, "file at the target throws") && passed;
    passed = check(std::filesystem::is_regular_file(temp / "file.txt"), "target file is still a file") && passed;
    passed = check(fileSystem.readBytes(temp / "file.txt") == minigit::FileBytes{'k', 'e', 'e', 'p'},
                   "target file is unchanged") && passed;

    // Error case: a regular file blocking a parent folder throws and stays unchanged.
    std::ofstream(temp / "blocked") << "keep";
    threw = false;
    try {
        fileSystem.createDirectories(temp / "blocked" / "b");
    } catch (const std::runtime_error&) {
        threw = true;
    }
    passed = check(threw, "file in the parent path throws") && passed;
    passed = check(std::filesystem::is_regular_file(temp / "blocked"), "blocking file is still a file") && passed;
    passed = check(fileSystem.readBytes(temp / "blocked") == minigit::FileBytes{'k', 'e', 'e', 'p'},
                   "blocking file is unchanged") && passed;

    std::filesystem::remove_all(temp);
    return passed;
}

int main() {
    // Returning 1 tells CTest a test failed; returning 0 means all checks passed.
    if (!testReadBytes()) return 1;
    if (!testWriteBytesSafely()) return 1;
    if (!testWriteErrors()) return 1;
    if (!testFailedReplacement()) return 1;
    if (!testRemove()) return 1;
    if (!testCopy()) return 1;
    if (!testCreateDirectories()) return 1;
    std::cout << "FileSystem tests passed\n";
    return 0;
}
