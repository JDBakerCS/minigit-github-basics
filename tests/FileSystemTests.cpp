#include "FileSystem.hpp"
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

int main() {
    if (!testRemove()) return 1;
    std::cout << "FileSystem tests passed\n";
    return 0;
}