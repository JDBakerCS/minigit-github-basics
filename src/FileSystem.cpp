#include "FileSystem.hpp"
#include <fstream>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace minigit {

bool FileSystem::isRegularFile(const std::filesystem::path& file) const {
    return std::filesystem::is_regular_file(file);
}

FileBytes FileSystem::readBytes(const std::filesystem::path& file) const {
    if (!isRegularFile(file)) {
        throw std::runtime_error("Cannot read file: " + file.string());
    }

    std::ifstream input(file, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Cannot open file: " + file.string());
    }

    FileBytes bytes;
    char byte;
    while (input.get(byte)) {
        bytes.push_back(static_cast<unsigned char>(byte));
    }
    if (input.bad() || !input.eof()) {
        throw std::runtime_error("Cannot read file: " + file.string());
    }
    return bytes;
}

void FileSystem::writeBytesSafely(const std::filesystem::path& target,
                                 const FileBytes& bytes) const {
    const auto parent = target.has_parent_path() ? target.parent_path()
                                                : std::filesystem::path(".");
    if (target.filename().empty() || !std::filesystem::is_directory(parent)) {
        throw std::runtime_error("Invalid target or missing parent: " + target.string());
    }

    // Reserve a workspace beside the target without overwriting existing files.
    auto workspace = target;
    workspace += ".tmp";
    if (!std::filesystem::create_directory(workspace)) {
        throw std::runtime_error("Temporary workspace already exists: " + workspace.string());
    }
    const auto temporary = workspace / "bytes";

    try {
        std::ofstream output(temporary, std::ios::binary);
        if (!output) {
            throw std::runtime_error("Cannot open temporary file: " + temporary.string());
        }
        for (unsigned char byte : bytes) {
            output.put(static_cast<char>(byte));
        }
        output.close();
        if (!output) {
            throw std::runtime_error("Cannot finish writing file: " + target.string());
        }

        // Replace only after writing and closing succeed. Never delete the target first.
#ifdef _WIN32
        if (!MoveFileExW(temporary.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING)) {
            const std::error_code error(GetLastError(), std::system_category());
            throw std::runtime_error("Cannot replace file: " + target.string() +
                                     ": " + error.message());
        }
#else
        std::filesystem::rename(temporary, target);
#endif
    } catch (...) {
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        std::filesystem::remove(workspace, ignored);
        throw;
    }

    // A cleanup failure must not report a successful replacement as a failed write.
    std::error_code ignored;
    std::filesystem::remove(workspace, ignored);
}

void FileSystem::remove(const std::filesystem::path& file) const {
    // Only deletes a real file, never a folder or a missing path.
    if (!std::filesystem::is_regular_file(file)) {
        throw std::runtime_error("Cannot remove file: " + file.string());
    }
    std::filesystem::remove(file);
}
    
void FileSystem::createDirectories(const std::filesystem::path& directory) const {
    if (std::filesystem::is_directory(directory)) {
        return;
    }
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error || !std::filesystem::is_directory(directory)) {
        throw std::runtime_error("Cannot create directory: " + directory.string());
    }
}
}