#pragma once
#include <filesystem>

namespace minigit {

class FileSystem {
public:
    void remove(const std::filesystem::path& file) const;
};

} 