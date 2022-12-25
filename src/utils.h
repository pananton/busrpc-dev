#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

/// \file utils.h Utilities.

namespace busrpc {

/// Turns token compression on or off for \ref SplitString function.
enum class TokenCompressMode { Off = 1, On = 2 };

/// Split \a str into a vector of tokens.
std::vector<std::string> SplitString(const std::string& str,
                                     char delimeter = '\n',
                                     TokenCompressMode mode = TokenCompressMode::Off);

/// Remove leading and trailing spaces/tabs from \a str.
std::string TrimString(const std::string& str);

/// Return \c true if \a name consists of alphas in lowercase, digits and underscores.
/// \note Digit can't be used as starting character.
/// \note Because empty string does not have any prohibited characters, this function returns \c true for it.
bool IsLowercaseWithUnderscores(std::string_view name);

/// Return \c true if \a name consists of alphas in uppercase, digits and underscores.
/// \note Digit can't be used as starting character.
/// \note Because empty string does not have any prohibited characters, this function returns \c true for it.
bool IsUppercaseWithUnderscores(std::string_view name);

/// Return \c true if \a name consists of lower- and uppercase alphas (with single initial uppercase alpha) and digits.
/// \note Digit can't be used as starting character.
/// \note Because empty string does not have any prohibited characters, this function returns \c true for it.
bool IsCamelCase(std::string_view name);

    /// Initialize \a path to a canonical path to the existing directory.
/// \throws std::filesystem::filesystem_error
/// \note Value of \a path is not changed if any error occurs.
/// \note If \a dir is empty, then canonical path to current directory is returned.
bool InitCanonicalPathToExistingDirectory(std::filesystem::path& path, const std::string& dir = {});

/// Initialize \a path to a relative path to the existing file.
/// \throws std::filesystem::filesystem_error
/// \note Function returns \c false if \a file does not exist, is not regular, or located outside the \a root.
/// \note Function guarantees, that returned path will not contain any '.' or '..' components.
/// \note Value of \a path is not modified if any error occurs.
bool InitRelativePathToExistingFile(std::filesystem::path& path,
                                    const std::string& file,
                                    const std::filesystem::path& root = std::filesystem::current_path());
} // namespace busrpc