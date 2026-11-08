#pragma once
#include <filesystem>

namespace minigit {
    class RepositoryPaths {
        public:
            explicit RepositoryPaths(std::filesystem::path projectRoot);

            const std::filesystem::path& projectRoot() const;
            const std::filesystem::path& metadataRoot() const;

            std::filesystem::path headFile() const;
            std::filesystem::path indexFile() const;

          private:
            std::filesystem::path projectRoot_;
            std::filesystem::path metadataRoot_;
    };
}