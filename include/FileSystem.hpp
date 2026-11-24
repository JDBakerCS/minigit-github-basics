#pragma once
#include <filesystem>
#include <vector>

namespace minigit {

using FileBytes = std::vector<unsigned char>;

class FileSystem {
public:
    bool isRegularFile(const std::filesystem::path& file) const;
    FileBytes readBytes(const std::filesystem::path& file) const;
    void writeBytesSafely(const std::filesystem::path& target,
                          const FileBytes& bytes) const;
    void copy(const std::filesystem::path& source,
              const std::filesystem::path& destination) const;
    void remove(const std::filesystem::path& file) const;
};

}