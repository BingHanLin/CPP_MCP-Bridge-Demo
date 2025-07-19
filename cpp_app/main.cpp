#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "grpcServerStrategy.hpp"
#include "socketServerStrategy.hpp"

int main(int argc, char** argv)
{
    try
    {
        auto server = std::make_unique<socketServerStrategy>(9876);

        // auto server = std::make_unique<grpcServerStrategy>("0.0.0.0:50051");

        std::cout << "========================================" << std::endl;
        std::cout << "  C++ MCP Bridge Demo" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "========================================" << std::endl;

        // Start the server (this will block)
        server->start();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
