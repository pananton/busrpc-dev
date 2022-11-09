#pragma once

#include <optional>
#include <stdexcept>

/// Base class for all application exceptions.
class busrpc_error: public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

/// Command arguments parsing error.
class args_error: public busrpc_error {
public:
    using busrpc_error::busrpc_error;
};
