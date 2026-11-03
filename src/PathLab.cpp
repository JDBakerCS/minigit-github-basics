#include "PathLab.hpp"
#include <fstream>
#include <system_error>

namespace minigit {

std::filesystem::path metadataDirectory(const std::filesystem::path &root) {
  return root / ".minigit";
}
std::filesystem::path headFilePath(const std::filesystem::path &root) {
  return metadataDirectory(root) / "HEAD";
}
WriteResult writeTextFile(const std::filesystem::path &file,
                          const std::string &text) {
    const std::filesystem::path parent = file.parent_path();

    if (!parent.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            return {false, "could not create directory " + parent.string() +
                               ": " + ec.message()};
        }
    }

    std::ofstream out(file, std::ios::binary | std::ios::trunc);
    if (!out) {
        return {false, "could not open file for writing: " + file.string()};
    }

    out << text;
    out.close();
    if (out.fail()) {
        return {false, "failed while writing file: " + file.string()};
    }

    return {true, ""};
}
} // namespace minigit
