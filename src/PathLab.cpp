#include "PathLab.hpp"
#include <fstream>
#include <system_error>

namespace minigit
{

    std::filesystem::path metadataDirectory(const std::filesystem::path &root)
    {
        return root / ".minigit";
    }
    std::filesystem::path headFilePath(const std::filesystem::path &root)
    {
        return metadataDirectory(root) / "HEAD";
    }
    WriteResult writeTextFile(const std::filesystem::path &file,
                              const std::string &text)
    {
        const std::filesystem::path parent = file.parent_path();

        if (!parent.empty())
        {
            std::error_code ec;
            std::filesystem::create_directories(parent, ec);
            if (ec)
            {
                return {false, "could not create directory " + parent.string() +
                                   ": " + ec.message()};
            }
        }

        std::ofstream out(file, std::ios::binary | std::ios::trunc);
        if (!out)
        {
            return {false, "could not open file for writing: " + file.string()};
        }

        out << text;
        out.close();
        if (out.fail())
        {
            return {false, "failed while writing file: " + file.string()};
        }

        return {true, ""};
    }

    ReadResult readTextFile(const std::filesystem::path &file)
    {
        // Binary mode preserves the exact newlines written by writeTextFile.
        std::ifstream in(file, std::ios::binary);
        if (!in)
        {
            return {false, "", "could not open file for reading: " + file.string()};
        }

        std::string text;
        char character;
        while (in.get(character))
        {
            text += character;
        }

        // Reaching the end is normal; other read failures must be reported.
        if (in.bad() || !in.eof())
        {
            return {false, "", "failed while reading file: " + file.string()};
        }

        return {true, text, ""};
    }

} // namespace minigit
