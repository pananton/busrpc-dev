#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

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
    /// \note Parameter \a ignoredCategories determines error code categories to be ignored by the collector.
    /// \note This constructor assumes that protobuf errors will not be collected and does not initialize protobuf
    ///       error collector.
    explicit ErrorCollector(SeverityOrder orderFunc = {},
                            std::vector<const std::error_category*> ignoredCategories = {});

    /// Create error collector for the error \a category.
    /// \note Function \a orderFunc is invoked for two error codes to determine which one should be considered more
    ///       severe. It is used to calculate the most severe error code which is returned by \ref majorError method.
    /// \note Parameter \a ignoredCategories determines error code categories to be ignored by the collector.
    /// \note This constructor assumes that protobuf errors will be collected with \a protobufErrorCode and
    ///       initializes protobuf error collector.
    ErrorCollector(std::error_code protobufErrorCode,
                   SeverityOrder orderFunc = {},
                   std::vector<const std::error_category*> ignoredCategories = {});

    /// Add \a ec to the stored errors and append all \a specifiers to the added error description.
    /// \tparam TSpecifiers Specifier types.
    /// \note If \a ec does not indicate an error or it's category is ignored, method does nothing.
    /// \note Specifiers are used to provide custom information about the error. To be used as a specifier,
    ///       type should support <tt>ostream& operator\<\<</tt>. Additionally, this method also accepts
    ///       \c std::pair as specifier type, which is converted to string <tt>\<first>=\<second></tt> when
    ///       building error description.
    template<typename... TSpecifiers>
    void add(std::error_code ec, const TSpecifiers&... specifiers) noexcept
    {
        if (!ec || isIgnored(&ec.category())) {
            return;
        }

        std::ostringstream out;
        OutputSpecifiers(out, specifiers...);
        std::string specifiersStr = out.str();

        std::string description =
            "[" + std::string(ec.category().name() ? ec.category().name() : "") + "] " + ec.message();

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

    /// Search for the first error with the specified \a ec.
    std::optional<ErrorInfo> find(std::error_code ec);

    /// Return \c true if collector contains error(s).
    explicit operator bool() const noexcept { return static_cast<bool>(majorError_); }

    /// Return collector for protobuf parsing errors.
    google::protobuf::compiler::MultiFileErrorCollector* getProtobufCollector() const noexcept
    {
        return protobufCollector_.get();
    }

private:
    ErrorCollector(std::error_code* protobufErrorCode,
                   SeverityOrder orderFunc,
                   std::vector<const std::error_category*> ignoredCategories);
    bool isIgnored(const std::error_category* category) const noexcept;

    template<typename TArg, typename... TArgs>
    static void OutputSpecifiers(std::ostream& out, const TArg& arg, const TArgs&... args);

    template<typename T, typename U, typename... TArgs>
    static void OutputSpecifiers(std::ostream& out, const std::pair<T, U>& arg, const TArgs&... args);

    static void OutputSpecifiers(std::ostream&) { }

    SeverityOrder orderFunc_;
    std::vector<const std::error_category*> ignoredCategories_;
    std::shared_ptr<google::protobuf::compiler::MultiFileErrorCollector> protobufCollector_;

    std::optional<ErrorInfo> majorError_;
    std::vector<ErrorInfo> errors_;
};

/// Output all error to the \a out stream.
std::ostream& operator<<(std::ostream& out, const ErrorCollector& collector);

/// Error collector guard, which provides guarantees that collected error information will be outputted.
class ErrorCollectorGuard {
public:
    /// Create guard.
    /// \warning Both \a collector and \a err should not be destroyed until guard is destroyed.
    ErrorCollectorGuard(const ErrorCollector& collector, std::ostream& err): ecol_(collector), err_(err) { }

    /// Destroy guard and output collected errors to specified error stream.
    ~ErrorCollectorGuard() { err_ << ecol_; }

    ErrorCollectorGuard(const ErrorCollectorGuard&) = delete;
    ErrorCollectorGuard(ErrorCollectorGuard&&) = delete;
    ErrorCollectorGuard& operator=(const ErrorCollectorGuard&) = delete;
    ErrorCollectorGuard& operator=(ErrorCollectorGuard&&) = delete;

private:
    const ErrorCollector& ecol_;
    std::ostream& err_;
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
} // namespace busrpc
