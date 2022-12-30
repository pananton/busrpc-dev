#pragma once

#include "error_collector.h"

#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4100)
#    pragma warning(disable : 4251)
#endif

#include <google/protobuf/compiler/importer.h>

#ifdef _MSC_VER
#    pragma warning(pop)
#endif

/// \file protobuf_error_collector.h Protobuf compiler error collector.

namespace busrpc {

/// Protobuf compiler error collector.
class ProtobufErrorCollector: public google::protobuf::compiler::MultiFileErrorCollector {
public:
    /// Create protobuf error collector.
    /// \note Errors found by protobuf library will be added to this collector, whihc in it's turn adds them to
    ///       \a collector with \a protobufErrorCode error code.
    ProtobufErrorCollector(ErrorCollector& collector, std::error_code protobufErrorCode):
        collector_(collector),
        protobufErrorCode_(std::move(protobufErrorCode))
    { }

    ProtobufErrorCollector(const ProtobufErrorCollector&) = delete;
    ProtobufErrorCollector(ProtobufErrorCollector&&) = delete;
    ProtobufErrorCollector& operator=(const ProtobufErrorCollector&) = delete;
    ProtobufErrorCollector& operator=(ProtobufErrorCollector&&) = delete;

    /// Called by protobuf library when it encounters error when parsing protocol file.
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
} // namespace busrpc