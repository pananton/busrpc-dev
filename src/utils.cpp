#include "utils.h"

#include <sstream>

namespace busrpc {

namespace {

bool islower(char c)
{
    return c >= 0x61 && c <= 0x7a;
}

bool isupper(char c)
{
    return c >= 0x41 && c <= 0x5a;
}

bool isdigit(char c)
{
    return c >= 0x30 && c <= 0x39;
}

bool isunder(char c)
{
    return c == 0x5f;
}
} // namespace

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

bool IsLowercaseWithUnderscores(std::string_view name)
{
    if (name.empty()) {
        return true;
    }

    if (isdigit(name[0])) {
        return false;
    }

    for (auto c: name) {
        if (!islower(c) && !isdigit(c) && !isunder(c)) {
            return false;
        }
    }

    return true;
}

bool IsUppercaseWithUnderscores(std::string_view name)
{
    if (name.empty()) {
        return true;
    }

    if (isdigit(name[0])) {
        return false;
    }

    for (auto c: name) {
        if (!isupper(c) && !isdigit(c) && !isunder(c)) {
            return false;
        }
    }

    return true;
}

bool IsCamelCase(std::string_view name)
{
    if (name.empty()) {
        return true;
    }

    if (isdigit(name[0]) || !isupper(name[0])) {
        return false;
    }

    bool prevCharIsUpper = false;

    for (auto c: name) {
        if (isupper(c)) {
            if (prevCharIsUpper) {
                return false;
            } else {
                prevCharIsUpper = true;
            }
        } else if (islower(c) || isdigit(c)) {
            prevCharIsUpper = false;
        } else {
            return false;
        }
    }

    return true;
}

bool InitCanonicalPathToExistingDirectory(std::filesystem::path& path, const std::string& dir)
{
    if (dir.empty()) {
        path = std::filesystem::current_path();
    } else {
        std::error_code ec;
        std::filesystem::path tmp = std::filesystem::weakly_canonical(dir, ec);

        if (!ec && std::filesystem::is_directory(tmp)) {
            path = tmp;
            return true;
        }

        return false;
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
