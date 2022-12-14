#pragma once

#include "utils.h"
#include "utils/file_utils.h"

#include <gtest/gtest.h>

#include <filesystem>

namespace busrpc { namespace test {

TEST(UtilsTest, SplitString_Splits_Empty_String_To_Zero_Tokens)
{
    EXPECT_TRUE(SplitString("", '\n', TokenCompressMode::Off).empty());
}

TEST(UtilsTest, SplitString_Splits_String_Wo_Delimiter_To_Single_Token)
{
    std::string str = "test string";
    auto result = SplitString(str, '-', TokenCompressMode::Off);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], str);
}

TEST(UtilsTest, SplitString_Splits_String_With_Multiple_Delimiters_To_Corresponding_Number_Of_Tokens)
{
    std::string str = "str1 str2 str3";
    auto result = SplitString(str, ' ');

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "str1");
    EXPECT_EQ(result[1], "str2");
    EXPECT_EQ(result[2], "str3");
}

TEST(UtilsTest, SplitString_Returns_Empty_Tokens_If_Token_Compression_Is_Off)
{
    std::string str = "\n\nline 1\n\nline 2\n\n";
    auto result = SplitString(str, '\n', TokenCompressMode::Off);

    ASSERT_EQ(result.size(), 6);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "line 1");
    EXPECT_EQ(result[3], "");
    EXPECT_EQ(result[4], "line 2");
    EXPECT_EQ(result[5], "");
}

TEST(UtilsTest, SplitString_Skips_Empty_Tokens_If_Token_Compression_Is_On)
{
    std::string str = "\n\nline 1\n\nline 2\n\n";
    auto result = SplitString(str, '\n', TokenCompressMode::On);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "line 1");
    EXPECT_EQ(result[1], "line 2");
}

TEST(UtilsTest, TrimString_Correctly_Handles_Empty_String)
{
    EXPECT_TRUE(TrimString("").empty());
}

TEST(UtilsTest, TrimString_Correctly_Handles_String_Consisting_Of_Whitespaces_Only)
{
    EXPECT_TRUE(TrimString(" ").empty());
    EXPECT_TRUE(TrimString("\t").empty());
    EXPECT_TRUE(TrimString("   \t \t").empty());
}

TEST(UtilsTest, TrimString_Correctly_Handles_String_Wo_Whitespaces)
{
    std::string str = "abc";

    EXPECT_EQ(str, TrimString(str));

    str = "a";

    EXPECT_EQ(str, TrimString(str));
}

TEST(UtilsTest, TrimString_Correctly_Handles_String_With_Whitespaces)
{
    EXPECT_EQ(TrimString("\tabc \t def \t "), "abc \t def");
}

TEST(UtilsTest, InitCanonicalPathToExistingDirectory_Returns_False_If_Dir_Does_Not_Exist)
{
    std::filesystem::path path;

    EXPECT_FALSE(InitCanonicalPathToExistingDirectory(path, "missing_directory"));
}

TEST(UtilsTest, InitCanonicalPathToExistingDirectory_Returns_False_If_Argument_Is_File)
{
    std::filesystem::path path;
    TmpDir tmp;
    tmp.writeFile("file.txt");

    EXPECT_FALSE(InitCanonicalPathToExistingDirectory(path, "tmp/file.txt"));
}

TEST(UtilsTest, InitCanonicalPathToExistingDirectory_Does_Not_Modify_Path_If_Any_Error_Occurs)
{
    std::filesystem::path path = "aaa";
    TmpDir tmp;
    tmp.writeFile("file.txt");

    EXPECT_FALSE(InitCanonicalPathToExistingDirectory(path, "missing_directory"));
    EXPECT_EQ(path, "aaa");

    EXPECT_FALSE(InitCanonicalPathToExistingDirectory(path, "tmp/file.txt"));
    EXPECT_EQ(path, "aaa");
}

TEST(UtilsTest, InitCanonicalPathToExistingDirectory_Initializes_Current_Path_By_Default)
{
    std::filesystem::path path;

    ASSERT_TRUE(InitCanonicalPathToExistingDirectory(path));
    EXPECT_EQ(path, std::filesystem::current_path());
}

TEST(UtilsTest, InitCanonicalPathToExistingDirectory_Initializes_Canonical_Path_If_Dir_Exists)
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

TEST(UtilsTest, InitRelativePathToExistingFile_Returns_False_If_Filename_Is_Empty)
{
    std::filesystem::path path;

    EXPECT_FALSE(InitRelativePathToExistingFile(path, ""));
    EXPECT_FALSE(InitRelativePathToExistingFile(path, "", "."));
}

TEST(UtilsTest, InitRelativePathToExistingFile_Returns_False_If_File_Does_Not_Exist)
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

TEST(UtilsTest, InitRelativePathToExistingFile_Returns_False_If_Argument_Is_Not_File)
{
    std::filesystem::path path;
    TmpDir tmp;
    tmp.writeFile("subdir/file.txt");

    EXPECT_FALSE(InitRelativePathToExistingFile(path, "tmp"));
    EXPECT_FALSE(InitRelativePathToExistingFile(path, "tmp/subdir"));
}

TEST(UtilsTest, InitRelativePathToExistingFile_Returns_False_If_File_Outside_Root_Is_Specified)
{
    std::filesystem::path path;
    TmpDir tmp;
    tmp.writeFile("file1.txt");
    tmp.writeFile("subdir/file2.txt");

    EXPECT_FALSE(InitRelativePathToExistingFile(path, "tmp/subdir/../file1.txt", "tmp/subdir"));
    EXPECT_FALSE(InitRelativePathToExistingFile(
        path, (std::filesystem::absolute(tmp.path()) / "file1.txt").string(), "tmp/subdir"));
}

TEST(UtilsTest, InitRelativePathToExistingFile_Does_Not_Modify_Path_If_Any_Error_Occurs)
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

TEST(UtilsTest, InitRelativePathToExistingFile_Returns_Relative_Path_Without_Leading_Dot)
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
