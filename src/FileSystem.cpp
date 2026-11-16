#include "FileSystem.hpp"
#include <stdexcept>

namespace minigit {

void FileSystem::remove(const std::filesystem::path& file) const {
    // Only deletes a real file, never a folder or a missing path.
    if (!std::filesystem::is_regular_file(file)) {
        throw std::runtime_error("Cannot remove file: " + file.string());
    }
    std::filesystem::remove(file);
}

} 