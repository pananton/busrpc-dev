#pragma once

#include <filesystem>
#include <string>

/// \file utils.h Utilities.

namespace busrpc {

/// Initialize \a path to a canonical path to the existing directory.
/// \throws std::filesystem::filesystem_error
/// \note Value of \a path is not changed if any error occurs.
/// \note If \a dir is empty, then canonical path to current directory is returned.
bool InitCanonicalPathToExistingDirectory(std::filesystem::path& path, const std::string& dir = "");

/// Initialize \a path to a relative path to the existing file.
/// \throws std::filesystem::filesystem_error
/// \note Function returns \c false if \a file does not exist, is not regular, or located outside the \a root.
/// \note Function guarantees, that returned path will not contain any '.' or '..' components.
/// \note Value of \a path is not modified if any error occurs.
bool InitRelativePathToExistingFile(std::filesystem::path& path,
                                    const std::string& file,
                                    const std::filesystem::path& root = std::filesystem::current_path());

} // namespace busrpc