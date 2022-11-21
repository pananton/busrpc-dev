#pragma once

#include <iostream>

/// \namespace busrpc Busrpc development tool namespace.
/// \file app.h Functions for managing busrpc command line application.

namespace CLI {
class App;
}

namespace busrpc {

/// Initializes busrpc development tool command line application.
void InitApp(CLI::App& app, std::ostream& out = std::cout , std::ostream& err = std::cerr);
} // namespace busrpc