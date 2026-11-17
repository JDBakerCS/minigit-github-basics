#include "FileSystem.hpp"
#include <fstream>
#include <stdexcept>

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

}
