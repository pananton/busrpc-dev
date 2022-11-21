#include "app.h"

#include <CLI/CLI.hpp>

#include <iostream>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    CLI::App app("Busrpc development tool", "busrpc");
    busrpc::InitApp(app);

    if (argc <= 1) {
        std::cout << app.help();
        return EXIT_SUCCESS;
    }

    try {
        app.parse(argc, argv);
        return EXIT_SUCCESS;
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "exception caught" << std::endl;
        return EXIT_FAILURE;
    }
}