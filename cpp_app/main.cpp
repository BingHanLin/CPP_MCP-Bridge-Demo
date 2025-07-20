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
        std::string mode = "socket";  // Default mode
        std::string address;

        // Parse command line arguments
        if (argc > 1)
        {
            mode = argv[1];
        }

        std::unique_ptr<serverStrategy> server;
        if (mode == "socket")
        {
            int port = 9876;
            if (argc > 2)
            {
                port = std::stoi(argv[2]);
            }
            address = std::to_string(port);
            server = std::make_unique<socketServerStrategy>(port);
        }
        else if (mode == "grpc")
        {
            address = "0.0.0.0:50051";
            if (argc > 2)
            {
                address = argv[2];
            }
            server = std::make_unique<grpcServerStrategy>(address);
        }
        else
        {
            std::cerr << "Usage: " << argv[0] << " [socket|grpc] [address]" << std::endl;
            std::cerr << "  socket mode: address is port number (default: 9876)" << std::endl;
            std::cerr << "  grpc mode: address is host:port (default: 0.0.0.0:50051)" << std::endl;
            return 1;
        }

        std::cout << "========================================" << std::endl;
        std::cout << "  C++ MCP Bridge Demo" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Mode: " << mode << std::endl;
        std::cout << "Address: " << address << std::endl;
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
