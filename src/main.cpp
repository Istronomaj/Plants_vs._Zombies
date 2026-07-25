#include "pvz/app/Application.hpp"

#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    try {
        pvz::app::Application app{pvz::app::parseArgs(argc, argv)};
        return app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << '\n';
        return 1;
    }
}
