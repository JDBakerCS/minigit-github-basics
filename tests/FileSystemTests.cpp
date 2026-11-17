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
        // Include a null byte, line endings, and values above 127 to test binary data.
        const minigit::FileBytes binary = {0, 10, 13, 26, 128, 255};
        fileSystem.writeBytesSafely(target, binary);
        // Keep an earlier failure in passed, while still running each new check.
        passed = check(fileSystem.readBytes(target) == binary, "write preserves binary bytes") && passed;
        // A shorter replacement must remove all bytes from the old version.
        fileSystem.writeBytesSafely(target, {42});
        passed = check(fileSystem.readBytes(target) == minigit::FileBytes{42}, "shorter replacement has no leftover bytes") && passed;
        // An empty byte vector should produce an existing file with zero bytes.
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

// Check errors that happen before the original file can be replaced.
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

        // Here an exception is expected: the function must not create the parent.
        bool threw = false;
        try {
            fileSystem.writeBytesSafely(temp / "missing" / "data.bin", binary);
        } catch (const std::runtime_error&) {
            threw = true;
        }
        passed = check(threw, "missing parent throws") && passed;
        passed = check(!std::filesystem::exists(temp / "missing"), "missing parent is not created") && passed;

        // Occupy the temporary workspace to simulate failure before replacement.
        // The function must leave this existing workspace and the original file alone.
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

// Check cleanup when temporary bytes are written but replacement cannot finish.
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

        // A directory cannot be replaced by a file; staged bytes must be cleaned up.
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

int main() {
    // Returning 1 tells CTest a test failed; returning 0 means all checks passed.
    if (!testReadBytes()) return 1;
    if (!testWriteBytesSafely()) return 1;
    if (!testWriteErrors()) return 1;
    if (!testFailedReplacement()) return 1;
    std::cout << "FileSystem tests passed\n";
    return 0;
}
