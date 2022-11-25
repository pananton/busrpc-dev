#include "utils/file_utils.h"

#include <filesystem>
#include <fstream>

namespace busrpc { namespace test {

void WriteFile(const std::filesystem::path& path, const std::string& content)
{
    if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream file;
    file.open(path.string());

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file '" + path.string() + "' for writing");
    }

    file << content;

    if (!file.good()) {
        throw std::runtime_error("failed to write file '" + path.string() + "'");
    }
}

std::string ReadFile(const std::filesystem::path& path)
{
    std::string content;
    std::ifstream file;
    file.open(path.string());

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file '" + path.string() + "' for readind");
    }

    file >> content;

    if (!file.good()) {
        throw std::runtime_error("failed to read file '" + path.string() + "'");
    }

    return content;
}

bool isNestedPath(const std::filesystem::path& root, const std::filesystem::path& path)
{
    auto normRoot = std::filesystem::canonical(root);
    auto normPath = std::filesystem::canonical(path);

    auto it = std::search(normPath.begin(), normPath.end(), normRoot.begin(), normRoot.end());
    return it == normPath.begin();
}

TmpDir::TmpDir(const std::string& path): dir_(path)
{
    if (!dir_.empty()) {
        std::filesystem::remove_all(dir_);
        std::filesystem::create_directories(dir_);
    }
}

TmpDir::~TmpDir()
{
    if (!dir_.empty()) {
        std::filesystem::remove_all(dir_);
    }
}

TmpDir::TmpDir(TmpDir&& rhs) noexcept: dir_(std::move(rhs.dir_))
{
    rhs.dir_.clear();
}

TmpDir& TmpDir::operator=(TmpDir&& rhs)
{
    if (!dir_.empty()) {
        std::filesystem::remove_all(dir_);
    }

    dir_ = std::move(rhs.dir_);
    rhs.dir_.clear();
    return *this;
}

const std::filesystem::path& TmpDir::path() const noexcept
{
    return dir_;
}

void TmpDir::writeFile(const std::filesystem::path& path, const std::string& content)
{
    auto filePath = path.is_relative() ? dir_ / path : path;
    WriteFile(filePath, content);

    if (!isNestedPath(dir_, filePath)) {
        std::filesystem::remove(filePath);
        throw std::runtime_error("path '" + path.string() + "' is outside temporary directory '" + dir_.string() + "'");
    }
}
}} // namespace busrpc::test