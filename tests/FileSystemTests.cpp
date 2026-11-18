#include "FileSystem.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

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
    const auto temp = std::filesystem::temp_directory_path() /
        ("fs1-tests-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    if (!std::filesystem::create_directory(temp)) {
        throw std::runtime_error("Cannot create test directory");
    }

    bool passed = true;
    try {
        const minigit::FileBytes binary = {0, 10, 13, 26, 127, 128, 255};
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

int main() {
    if (!testReadBytes()) return 1;
    if (!testRemove()) return 1;
    std::cout << "FileSystem tests passed\n";
    return 0;
}