#pragma once

#include <filesystem>
#include <string>

namespace busrpc { namespace test {

void WriteFile(const std::string& path, const std::string& content);
std::string ReadFile(const std::string& path);
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
    void writeFile(const std::filesystem::path&, const std::string& content = {});

private:
    std::filesystem::path dir_;
};
}} // namespace busrpc::test
