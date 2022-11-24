#pragma once

#include "utils.h"
#include "utils/file_utils.h"

#include <gtest/gtest.h>

#include <filesystem>

namespace busrpc { namespace test {

TEST(InitCanonicalPathToExistingDirectoryTest, Returns_False_If_Dir_Does_Not_Exist)
{
    std::filesystem::path path;

    EXPECT_FALSE(InitCanonicalPathToExistingDirectory(path, "missing_directory"));
}

TEST(InitCanonicalPathToExistingDirectoryTest, Returns_False_If_Argument_Is_File)
{
    std::filesystem::path path;
    TmpDir tmp;
    tmp.writeFile("file.txt");

    EXPECT_FALSE(InitCanonicalPathToExistingDirectory(path, "tmp/file.txt"));
}

TEST(InitCanonicalPathToExistingDirectoryTest, Path_Is_Not_Modified_If_Any_Error_Occurs)
{
    std::filesystem::path path = "aaa";
    TmpDir tmp;
    tmp.writeFile("file.txt");

    EXPECT_FALSE(InitCanonicalPathToExistingDirectory(path, "missing_directory"));
    EXPECT_EQ(path, "aaa");

    EXPECT_FALSE(InitCanonicalPathToExistingDirectory(path, "tmp/file.txt"));
    EXPECT_EQ(path, "aaa");
}

TEST(InitCanonicalPathToExistingDirectoryTest, Initializes_Current_Path_By_Default)
{
    std::filesystem::path path;

    ASSERT_TRUE(InitCanonicalPathToExistingDirectory(path));
    EXPECT_EQ(path, std::filesystem::current_path());
}

TEST(InitCanonicalPathToExistingDirectoryTest, Weakly_initializes_Canonical_Path_If_Dir_Exists)
{
    std::filesystem::path path;
    TmpDir tmp;
    tmp.writeFile("file1.txt");
    tmp.writeFile("subdir/file2.txt");

    ASSERT_TRUE(InitCanonicalPathToExistingDirectory(
        path, std::filesystem::current_path().string() + "/tmp/./subdir/xxx/../../../tmp"));
    EXPECT_TRUE(path.is_absolute());
    EXPECT_TRUE(std::filesystem::is_regular_file(path / "file1.txt"));

    ASSERT_TRUE(InitCanonicalPathToExistingDirectory(path, "tmp/./subdir/xxx/../../../tmp/subdir"));
    EXPECT_TRUE(path.is_absolute());
    EXPECT_TRUE(std::filesystem::is_regular_file(path / "file2.txt"));
}

TEST(InitRelativePathToExistingFile, Returns_False_If_Filename_Is_Empty)
{
    std::filesystem::path path;

    EXPECT_FALSE(InitRelativePathToExistingFile(path, ""));
    EXPECT_FALSE(InitRelativePathToExistingFile(path, "", "."));
}

TEST(InitRelativePathToExistingFile, Returns_False_If_File_Does_Not_Exist)
{
    std::filesystem::path path;

    EXPECT_FALSE(InitRelativePathToExistingFile(path, "missing_dir/missing_file.txt", "missing_dir"));
    EXPECT_FALSE(InitRelativePathToExistingFile(
        path, "missing_dir/missing_file.txt", (std::filesystem::current_path() / "missing_dir").string()));
    EXPECT_FALSE(InitRelativePathToExistingFile(
        path, (std::filesystem::current_path() / "missing_dir/missing_file.txt").string(), "missing_dir"));
    EXPECT_FALSE(
        InitRelativePathToExistingFile(path,
                                       (std::filesystem::current_path() / "missing_dir/missing_file.txt").string(),
                                       (std::filesystem::current_path() / "missing_dir").string()));

    EXPECT_FALSE(InitRelativePathToExistingFile(path, "missing_file.txt", "."));
    EXPECT_FALSE(InitRelativePathToExistingFile(path, "missing_file.txt", std::filesystem::current_path().string()));
    EXPECT_FALSE(
        InitRelativePathToExistingFile(path, (std::filesystem::current_path() / "missing_file.txt").string(), "."));
    EXPECT_FALSE(InitRelativePathToExistingFile(path, (std::filesystem::current_path() / "missing_file.txt").string()));
}

TEST(InitRelativePathToExistingFile, Returns_False_If_Argument_Is_Not_File)
{
    std::filesystem::path path;
    TmpDir tmp;
    tmp.writeFile("subdir/file.txt");

    EXPECT_FALSE(InitRelativePathToExistingFile(path, "tmp"));
    EXPECT_FALSE(InitRelativePathToExistingFile(path, "tmp/subdir"));
}

TEST(InitRelativePathToExistingFile, Returns_False_If_File_Outside_Root_Is_Specified)
{
    std::filesystem::path path;
    TmpDir tmp;
    tmp.writeFile("file1.txt");
    tmp.writeFile("subdir/file2.txt");

    EXPECT_FALSE(InitRelativePathToExistingFile(path, "tmp/subdir/../file1.txt", "tmp/subdir"));
    EXPECT_FALSE(InitRelativePathToExistingFile(
        path, (std::filesystem::absolute(tmp.path()) / "file1.txt").string(), "tmp/subdir"));
}

TEST(InitRelativePathToExistingFile, Path_Is_Not_Modified_If_Any_Error_Occurs)
{
    std::filesystem::path path = "original";
    TmpDir tmp;
    tmp.writeFile("file1.txt");
    tmp.writeFile("subdir/file2.txt");

    EXPECT_FALSE(InitRelativePathToExistingFile(path, "missing_file.txt"));
    EXPECT_EQ(path, "original");

    EXPECT_FALSE(InitRelativePathToExistingFile(path, "tmp/subdir"));
    EXPECT_EQ(path, "original");

    EXPECT_FALSE(InitRelativePathToExistingFile(path, "tmp/file1.txt", "tmp/subdir"));
    EXPECT_EQ(path, "original");
}

TEST(InitRelativePathToExistingFile, Returns_Relative_Path_Without_Leading_Dot)
{
    std::filesystem::path path;
    TmpDir tmp;
    tmp.writeFile("file1.txt");
    tmp.writeFile("subdir/file2.txt");

    ASSERT_TRUE(InitRelativePathToExistingFile(path, "file1.txt", std::filesystem::absolute(tmp.path()).string()));
    EXPECT_TRUE(path.is_relative());
    EXPECT_EQ(path, "file1.txt");

    path.clear();

    ASSERT_TRUE(InitRelativePathToExistingFile(path, "subdir/file2.txt", "tmp"));
    EXPECT_TRUE(path.is_relative());
    EXPECT_EQ(path, "subdir/file2.txt");

    path.clear();

    ASSERT_TRUE(InitRelativePathToExistingFile(path, "./tmp/subdir/.././file1.txt", "."));
    EXPECT_TRUE(path.is_relative());
    EXPECT_EQ(path, "tmp/file1.txt");

    path.clear();

    ASSERT_TRUE(InitRelativePathToExistingFile(
        path, std::filesystem::current_path().string() + "/tmp/./subdir/xxx/yyy/../../file2.txt"));
    EXPECT_TRUE(path.is_relative());
    EXPECT_EQ(path, "tmp/subdir/file2.txt");
}
}} // namespace busrpc::test
