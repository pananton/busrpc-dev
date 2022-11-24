#include "error_collector.h"

#include <google/protobuf/compiler/importer.h>

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

    void AddError(const std::string& filename, int line, int column, const std::string& message) override
    {
        std::string result(filename);

        if (line != -1) {
            result.append(1, ':');
            result.append(std::to_string(line));
            result.append(1, ':');
            result.append(std::to_string(column));
        }

        if (!message.empty()) {
            result.append(": ");
            result.append(message);
        }

        collector_.add(protobufErrorCode_, std::move(result));
    }

private:
    ErrorCollector& collector_;
    std::error_code protobufErrorCode_;
};
} // namespace

ErrorCollector::ErrorCollector(const std::error_category& category, std::ostream& err):
    ErrorCollector(category, nullptr, err)
{ }

ErrorCollector::ErrorCollector(const std::error_category& category,
                               std::error_code protobufErrorCode,
                               std::ostream& err):
    ErrorCollector(category, &protobufErrorCode, err)
{ }

void ErrorCollector::add(std::error_code ec, std::optional<std::string> msg) noexcept
{
    if (!ec || ec.category() != category_) {
        return;
    }

    if (msg) {
        err_ << *msg;
    } else {
        err_ << ec.message();
    }

    err_ << std::endl;

    if (ec.value() > result_.value()) {
        result_ = ec;
    }
}

std::error_code ErrorCollector::result() const noexcept
{
    return result_;
}

compiler::MultiFileErrorCollector* ErrorCollector::getProtobufCollector() const noexcept
{
    return protobufCollector_.get();
}

ErrorCollector::ErrorCollector(const std::error_category& category,
                               std::error_code* protobufErrorCode,
                               std::ostream& err):
    category_(category),
    err_(err),
    result_(0, category_),
    protobufCollector_(protobufErrorCode ? std::make_shared<ProtobufErrorCollector>(*this, *protobufErrorCode)
                                         : nullptr)
{ }
} // namespace busrpc
