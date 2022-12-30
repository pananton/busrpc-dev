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
    auto normRoot = std::filesystem::weakly_canonical(root);
    auto normPath = std::filesystem::weakly_canonical(path);

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

void TmpDir::writeFile(const std::filesystem::path& file, const std::string& content)
{
    if (!file.is_relative()) {
        throw std::runtime_error("should be relative path");
    }

    auto filePath = dir_ / file;
    WriteFile(filePath, content);
}

void TmpDir::createDir(const std::filesystem::path& dir)
{
    if (!dir.is_relative()) {
        throw std::runtime_error("should be relative path");
    }

    auto dirPath = dir_ / dir;
    std::filesystem::create_directories(dirPath);
}
}} // namespace busrpc::test
