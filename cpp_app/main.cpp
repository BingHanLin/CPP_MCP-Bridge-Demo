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
        auto server = std::make_unique<socketServerStrategy>();
        auto address = "9876";

        // auto server = std::make_unique<grpcServerStrategy>();
        // auto address = "0.0.0.0:50051";

        std::cout << "========================================" << std::endl;
        std::cout << "  C++ MCP Bridge Demo" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Address: " << address << std::endl;
        std::cout << "========================================" << std::endl;

        // Start the server (this will block)
        server->start(address);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
