#include "utils.h"

#include <sstream>

namespace busrpc {

std::vector<std::string> SplitString(const std::string& str, char delimiter, TokenCompressMode mode)
{
    std::stringstream s(str);
    std::string line;
    std::vector<std::string> result;

    while (std::getline(s, line, delimiter)) {
        if (!line.empty() || mode == TokenCompressMode::Off) {
            result.push_back(std::move(line));
        }
    }

    return result;
}

std::string TrimString(const std::string& str)
{
    constexpr const char* whitespace = " \t";
    auto start = str.find_first_not_of(whitespace);
    return start != std::string::npos ? str.substr(start, (str.find_last_not_of(whitespace) - start) + 1) : "";
}

bool InitCanonicalPathToExistingDirectory(std::filesystem::path& path, const std::string& dir)
{
    if (dir.empty()) {
        path = std::filesystem::current_path();
    } else {
        std::filesystem::path tmp(dir);

        if (!std::filesystem::is_directory(tmp)) {
            return false;
        }

        path = std::filesystem::canonical(tmp);
    }

    return true;
}

bool InitRelativePathToExistingFile(std::filesystem::path& path,
                                    const std::string& file,
                                    const std::filesystem::path& root)
{
    std::filesystem::path original(file);
    std::filesystem::path relPath;

    if (!original.parent_path().empty()) {
        relPath = std::filesystem::relative(root / original.parent_path(), root) / original.filename();
    } else {
        relPath = original.filename();
    }

    if (relPath.empty() || *relPath.begin() == "..") {
        return false;
    }

    if (!std::filesystem::is_regular_file(root / relPath)) {
        return false;
    }

    path = std::move(relPath);
    return true;
}
} // namespace busrpc
