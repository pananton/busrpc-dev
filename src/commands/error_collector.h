#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <system_error>

// \file error_collector.h Class for managing multiple errors.

namespace google { namespace protobuf { namespace compiler {
class MultiFileErrorCollector;
}}} // namespace google::protobuf::compiler

namespace busrpc {

/// Collects errors occurred during command execution and outputs them to error stream.
/// \warning Class expects, that all added errors share the same error category. Otherwise behaviour is unspecified.
class ErrorCollector {
public:
    /// Create error collector for the error \a category.
    /// \warning Error stream should not be destroyed while collector is used.
    /// \note This constructor assumes that protobuf errors will not be collected and does not initialize protobuf
    ///       error collector.
    ErrorCollector(const std::error_category& category, std::ostream& err = std::cerr);

    /// Create error collector for the error \a category.
    /// \warning Error stream should not be destroyed while collector is used.
    /// \note This constructor assumes that protobuf errors will be collected with \a protobufErrorCode and
    ///       initializes protobuf error collector.
    ErrorCollector(const std::error_category& category,
                   std::error_code protobufErrorCode,
                   std::ostream& err = std::cerr);

    /// Output information about \a ec to the error stream and update final error code if necessary.
    /// \warning If \a ec does not indicate an error, or it's category does not match the one specified in the
    ///          constructor, then added error code is completely ignored.
    /// \note If optional \a msg is set, then it is appended to the error code's description obtained from it's
    ///       error category.
    void add(std::error_code ec, std::optional<std::string> msg = std::nullopt) noexcept;

    /// Return final result.
    /// \note Result is the most severe error among collected. Note, that commands follow the rule, that more
    ///       severe errors should have higher numerical code (see \ref CommandError notes).
    std::error_code result() const noexcept;

    /// Return collector for protobuf parsing errors.
    google::protobuf::compiler::MultiFileErrorCollector* getProtobufCollector() const noexcept;

private:
    ErrorCollector(const std::error_category& category,
                   std::error_code* protobufErrorCode,
                   std::ostream& err);

    const std::error_category& category_;
    std::ostream& err_;
    std::error_code result_;
    std::shared_ptr<google::protobuf::compiler::MultiFileErrorCollector> protobufCollector_;
};
} // namespace busrpc
