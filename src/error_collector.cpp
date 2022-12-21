#include "error_collector.h"

#include <google/protobuf/compiler/importer.h>

#include <algorithm>

namespace compiler = google::protobuf::compiler;

namespace busrpc {
namespace {

class ProtobufErrorCollector: public compiler::MultiFileErrorCollector {
public:
    ProtobufErrorCollector(ErrorCollector& collector, std::error_code protobufErrorCode):
        collector_(collector),
        protobufErrorCode_(std::move(protobufErrorCode))
    { }

    ProtobufErrorCollector(const ProtobufErrorCollector&) = delete;
    ProtobufErrorCollector(ProtobufErrorCollector&&) = delete;
    ProtobufErrorCollector& operator=(const ProtobufErrorCollector&) = delete;
    ProtobufErrorCollector& operator=(ProtobufErrorCollector&&) = delete;

    void AddError(const std::string& filename, int line, int column, const std::string& description) override
    {
        collector_.add(protobufErrorCode_,
                       std::make_pair("file", filename),
                       std::make_pair("line", line),
                       std::make_pair("column", column),
                       std::make_pair("description", description));
    }

private:
    ErrorCollector& collector_;
    std::error_code protobufErrorCode_;
};
} // namespace

ErrorCollector::ErrorCollector(SeverityOrder orderFunc, std::vector<const std::error_category*> ignoredCategories):
    ErrorCollector(nullptr, std::move(orderFunc), std::move(ignoredCategories))
{ }

ErrorCollector::ErrorCollector(std::error_code protobufErrorCode,
                               SeverityOrder orderFunc,
                               std::vector<const std::error_category*> ignoredCategories):
    ErrorCollector(&protobufErrorCode, std::move(orderFunc), std::move(ignoredCategories))
{ }

ErrorCollector::ErrorCollector(std::error_code* protobufErrorCode,
                               SeverityOrder orderFunc,
                               std::vector<const std::error_category*> ignoredCategories):
    orderFunc_(std::move(orderFunc)),
    ignoredCategories_(std::move(ignoredCategories)),
    protobufCollector_(protobufErrorCode ? std::make_shared<ProtobufErrorCollector>(*this, *protobufErrorCode)
                                         : nullptr)
{ }

std::optional<ErrorCollector::ErrorInfo> ErrorCollector::find(std::error_code ec)
{
    auto it = std::find_if(errors_.begin(), errors_.end(), [&ec](const auto& info) { return info.code == ec; });
    return it != errors_.end() ? std::optional<ErrorInfo>(*it) : std::nullopt;
}

bool ErrorCollector::isIgnored(const std::error_category* category) const noexcept
{
    auto it = std::find_if(ignoredCategories_.begin(),
                           ignoredCategories_.end(),
                           [category](const auto& ignoredCategory) { return category == ignoredCategory; });
    return it != ignoredCategories_.end();
}

bool SeverityByErrorCodeValue(std::error_code lhs, std::error_code rhs)
{
    return lhs.value() < rhs.value();
}

std::ostream& operator<<(std::ostream& out, const ErrorCollector& collector)
{
    for (const auto& error: collector.errors()) {
        out << error.description << std::endl;
    }

    return out;
}
} // namespace busrpc
