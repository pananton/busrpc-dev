#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>

// \file error_collector.h Class for collecting multiple errors.

namespace google { namespace protobuf { namespace compiler {
class MultiFileErrorCollector;
}}} // namespace google::protobuf::compiler

namespace busrpc {

/// Type of the function used to order error codes by their severities.
/// \note If returns \c true, then \a lhs should be considered less severe than \a rhs and \c false otherwise (or
///       if \a lhs and \a rhs have the same severity level).
using SeverityOrder = std::function<bool(std::error_code lhs, std::error_code rhs)>;

/// Commonly used severity order functions, which treats error codes with greater value as being more severe.
bool SeverityByErrorCodeValue(std::error_code lhs, std::error_code rhs);

/// Collects multiple errors.
class ErrorCollector {
public:
    /// Information about error.
    struct ErrorInfo {
        std::error_code code;
        std::string description;
    };

    /// Create error collector.
    /// \note Function \a orderFunc is invoked for two error codes to determine which one should be considered more
    ///       severe. It is used to calculate the most severe error code which is returned by \ref majorError method.
    /// \note This constructor assumes that protobuf errors will not be collected and does not initialize protobuf
    ///       error collector.
    explicit ErrorCollector(SeverityOrder orderFunc = {});

    /// Create error collector for the error \a category.
    /// \note Function \a orderFunc is invoked for two error codes to determine which one should be considered more
    ///       severe. It is used to calculate the most severe error code which is returned by \ref majorError method.
    /// \note This constructor assumes that protobuf errors will be collected with \a protobufErrorCode and
    ///       initializes protobuf error collector.
    ErrorCollector(std::error_code protobufErrorCode, SeverityOrder orderFunc = {});

    /// Add \a ec to the stored errors and append all \a specifiers to the added error description.
    /// \tparam TSpecifiers Specifier types.
    /// \warning If \a ec does not indicate an error, or it's category does not match the one specified in the
    ///          constructor, then added error code is completely ignored.
    /// \note Specifiers are used to provide custom information about the error. To be used as a specifier,
    ///       type should support <tt>ostream& operator\<\<</tt>. Additionally, this method also accepts
    ///       \c std::pair as specifier type, which is converted to string <tt>\<first>=\<second></tt> when
    ///       building error description.
    template<typename... TSpecifiers>
    void add(std::error_code ec, const TSpecifiers&... specifiers) noexcept
    {
        if (!ec) {
            return;
        }

        std::ostringstream out;
        OutputSpecifiers(out, specifiers...);
        std::string specifiersStr = out.str();

        std::string description = ec.message();

        if (!specifiersStr.empty()) {
            description.append(": ");
            description.append(specifiersStr);
        }

        errors_.emplace_back(std::move(ec), std::move(description));

        if (!majorError_ || (orderFunc_ && orderFunc_(majorError_->code, ec))) {
            majorError_ = errors_.back();
        }
    }

    /// Clear all added errors.
    void clear() noexcept
    {
        majorError_.reset();
        errors_.clear();
    }

    /// Return the most severe error or \c nullopt if no error was added.
    /// \note The most severe error code is determined using \ref SeverityOrder function (see class' constructor).
    const std::optional<ErrorInfo>& majorError() const noexcept { return majorError_; }

    /// Return all errors in the order they were added to the collector.
    const std::vector<ErrorInfo>& errors() const noexcept { return errors_; }

    /// Return \c true if collector contains error(s).
    explicit operator bool() const noexcept { return static_cast<bool>(majorError_); }

    /// Return collector for protobuf parsing errors.
    google::protobuf::compiler::MultiFileErrorCollector* getProtobufCollector() const noexcept
    {
        return protobufCollector_.get();
    }

private:
    ErrorCollector(std::error_code* protobufErrorCode, SeverityOrder orderFunc);

    template<typename TArg, typename... TArgs>
    static void OutputSpecifiers(std::ostream& out, const TArg& arg, const TArgs&... args);

    template<typename T, typename U, typename... TArgs>
    static void OutputSpecifiers(std::ostream& out, const std::pair<T, U>& arg, const TArgs&... args);

    static void OutputSpecifiers(std::ostream&) { }

    SeverityOrder orderFunc_;
    std::shared_ptr<google::protobuf::compiler::MultiFileErrorCollector> protobufCollector_;

    std::optional<ErrorInfo> majorError_;
    std::vector<ErrorInfo> errors_;
};

template<typename TArg, typename... TArgs>
void ErrorCollector::OutputSpecifiers(std::ostream& out, const TArg& arg, const TArgs&... args)
{
    out << arg;

    if constexpr (sizeof...(args) != 0) {
        out << ", ";
    }

    OutputSpecifiers(out, args...);
}

template<typename T, typename U, typename... TArgs>
void ErrorCollector::OutputSpecifiers(std::ostream& out, const std::pair<T, U>& arg, const TArgs&... args)
{
    out << arg.first << "='" << arg.second << "'";

    if constexpr (sizeof...(args) != 0) {
        out << ", ";
    }

    OutputSpecifiers(out, args...);
}

/// Output all error to the \a out stream.
std::ostream& operator<<(std::ostream& out, const ErrorCollector& collector);
} // namespace busrpc
