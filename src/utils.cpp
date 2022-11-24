#include "utils.h"

namespace busrpc {

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
