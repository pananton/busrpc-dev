#pragma once

#include <filesystem>
#include <string>

namespace busrpc { namespace test {

void WriteFile(const std::filesystem::path& path, const std::string& content);
std::string ReadFile(const std::filesystem::path& path);
bool isNestedPath(const std::filesystem::path& root, const std::filesystem::path& path);

class TmpDir {
public:
    TmpDir(const std::string& path = "tmp");
    ~TmpDir();

    TmpDir(TmpDir&& rhs) noexcept;
    TmpDir& operator=(TmpDir&& rhs);

    TmpDir(const TmpDir&) = delete;
    TmpDir& operator=(const TmpDir&&) = delete;

    const std::filesystem::path& path() const noexcept;
    void writeFile(const std::filesystem::path& file, const std::string& content = {});
    void createDir(const std::filesystem::path& dir);

private:
    std::filesystem::path dir_;
};
}} // namespace busrpc::test
