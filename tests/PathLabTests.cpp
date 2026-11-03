#include "PathLab.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <system_error>

namespace
{
    struct TempDir
    {
        std::filesystem::path path;
        explicit TempDir(const std::filesystem::path &p) : path(p)
        {
            std::error_code ec;
            std::filesystem::remove_all(path, ec);
            std::filesystem::create_directories(path);
        }
        ~TempDir()
        {
            std::error_code ec;
            std::filesystem::remove_all(path, ec);
        }
    };

    std::string readAll(const std::filesystem::path &file)
    {
        std::ifstream in(file, std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(in),
                           std::istreambuf_iterator<char>());
    }
}
int main()
{

    std::filesystem::path sample = "course/repo";
    if (sample.filename() != "repo")
    {
        std::cerr << "path smoke check failed\n";
        return 1;
    }

    const std::filesystem::path ordinaryRoot = "repo";
    const std::filesystem::path nestedRoot =
        std::filesystem::path("course") / "repo";

    if (minigit::metadataDirectory(ordinaryRoot) !=
        ordinaryRoot / ".minigit")
    {
        std::cerr << "metadata directory path failed for ordinary root\n";
        return 1;
    }
    if (minigit::metadataDirectory(nestedRoot) !=
        nestedRoot / ".minigit")
    {
        std::cerr << "metadata directory path failed for nested root\n";
        return 1;
    }

    if (minigit::headFilePath(ordinaryRoot) !=
        ordinaryRoot / ".minigit" / "HEAD")
    {
        std::cerr << "HEAD file path failed for ordinary root\n";
        return 1;
    }
    if (minigit::headFilePath(nestedRoot) !=
        nestedRoot / ".minigit" / "HEAD")
    {
        std::cerr << "HEAD file path failed for nested root\n";
        return 1;
    }

    {
        TempDir temp(std::filesystem::temp_directory_path() /
                     "minigit_write_text_test");
        const std::string text = "line one\nline two\n";

        const std::filesystem::path nestedFile =
            temp.path / "a" / "b" / "c" / "out.txt";
        if (std::filesystem::exists(nestedFile.parent_path()))
        {
            std::cerr << "nested folder should start missing\n";
            return 1;
        }

        const minigit::WriteResult written =
            minigit::writeTextFile(nestedFile, text);
        if (!written.ok)
        {
            std::cerr << "write into new nested directory failed: "
                      << written.message << "\n";
            return 1;
        }
        if (!std::filesystem::exists(nestedFile) ||
            readAll(nestedFile) != text)
        {
            std::cerr << "written file is missing or has the wrong text\n";
            return 1;
        }

        // A regular file where a folder is needed must be reported, not ignored.
        const std::filesystem::path blocker = temp.path / "blocker";
        {
            std::ofstream(blocker) << "I am a file, not a folder";
        }
        const minigit::WriteResult failed =
            minigit::writeTextFile(blocker / "child.txt", "never written");
        if (failed.ok || failed.message.empty())
        {
            std::cerr << "write failure was not reported\n";
            return 1;
        }
    }
    if (std::filesystem::exists(std::filesystem::temp_directory_path() /
                                "minigit_write_text_test"))
    {
        std::cerr << "temporary test folder was not cleaned up\n";
        return 1;
    }

    {
        TempDir temp(std::filesystem::temp_directory_path() /
                     "minigit_read_text_test");
        const std::filesystem::path file = temp.path / "text.txt";
        const std::string text = "line one\nline two\r\nlast line";
        const minigit::WriteResult written = minigit::writeTextFile(file, text);
        if (!written.ok)
        {
            std::cerr << "read test setup failed: " << written.message << "\n";
            return 1;
        }

        const minigit::ReadResult read = minigit::readTextFile(file);
        if (!read.ok || read.text != text || !read.message.empty())
        {
            std::cerr << "reading exact contents failed: " << read.message << "\n";
            return 1;
        }

        const minigit::ReadResult missing =
            minigit::readTextFile(temp.path / "missing.txt");
        if (missing.ok || missing.message.empty() || !missing.text.empty())
        {
            std::cerr << "missing file failure was not reported\n";
            return 1;
        }

        const minigit::WriteResult emptied = minigit::writeTextFile(file, "");
        if (!emptied.ok)
        {
            std::cerr << "empty file setup failed: " << emptied.message << "\n";
            return 1;
        }
        const minigit::ReadResult empty = minigit::readTextFile(file);
        if (!empty.ok || !empty.text.empty() || !empty.message.empty())
        {
            std::cerr << "reading an empty file failed\n";
            return 1;
        }
    }
    if (std::filesystem::exists(std::filesystem::temp_directory_path() /
                                "minigit_read_text_test"))
    {
        std::cerr << "read test folder was not cleaned up\n";
        return 1;
    }

    std::cout << "PathLab path and file checks passed\n";

    return 0;
}
