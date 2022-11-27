#pragma once

#include <string>

/// \file lang_options.h Configuration options for target languages.

namespace busrpc {

// Options which determine protobuf file configuration options for java language.
struct JavaOptions {
    /// Value for the protobuf \c java_package option.
    /// \note If empty, then corresponding protobuf option is not added to file.
    std::string package = "";

    /// Value for the protobuf \c java_outer_classname.
    /// \note If empty, then corresponding protobuf option is not added to file.
    std::string outerClassName = "";

    /// Value for the protobuf \c java_multiple_files option.
    /// \note If false, then corresponding protobuf option is not added to file.
    bool multipleFiles = false;
};
} // namespace busrpc
