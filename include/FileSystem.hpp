#pragma once
#include <filesystem>
#include <vector>

namespace minigit {

using FileBytes = std::vector<unsigned char>;

class FileSystem {
public:
    bool isRegularFile(const std::filesystem::path& file) const;
    FileBytes readBytes(const std::filesystem::path& file) const;
    void remove(const std::filesystem::path& file) const;
};

}
