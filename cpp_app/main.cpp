#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>


#include "serverStrategy.hpp"
#include "socketServerStrategy.hpp"

// Conditional include for gRPC
#ifdef ENABLE_GRPC
#include "grpcServerStrategy.hpp"
#endif

void printUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " [server_type] [address]" << std::endl;
    std::cout << "  server_type: socket or grpc (default: socket)" << std::endl;
    std::cout << "  address: " << std::endl;
    std::cout << "    - For socket: port number or host:port (default: 9876)" << std::endl;
    std::cout << "    - For grpc: host:port (default: 0.0.0.0:50051)" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << "                    # Socket server on port 9876" << std::endl;
    std::cout << "  " << programName << " socket 8080        # Socket server on port 8080" << std::endl;
    std::cout << "  " << programName << " socket localhost:9000  # Socket server on localhost:9000" << std::endl;
#ifdef ENABLE_GRPC
    std::cout << "  " << programName << " grpc               # gRPC server on 0.0.0.0:50051" << std::endl;
    std::cout << "  " << programName << " grpc localhost:8080   # gRPC server on localhost:8080" << std::endl;
#endif
}

std::unique_ptr<serverStrategy> createServerStrategy(const std::string& serverType)
{
    if (serverType == "socket")
    {
        return std::make_unique<socketServerStrategy>();
    }
#ifdef ENABLE_GRPC
    else if (serverType == "grpc")
    {
        return std::make_unique<grpcServerStrategy>();
    }
#endif
    else
    {
        throw std::invalid_argument("Unknown server type: " + serverType +
#ifdef ENABLE_GRPC
                                    ". Supported types: socket, grpc"
#else
                                    ". Supported types: socket (grpc disabled)"
#endif
        );
    }
}

std::string getDefaultAddress(const std::string& serverType)
{
    if (serverType == "socket")
    {
        return "9876";
    }
    else if (serverType == "grpc")
    {
        return "0.0.0.0:50051";
    }
    return "9876";
}

int main(int argc, char** argv)
{
    std::string serverType = "socket";  // Default to socket
    std::string address;

    // Parse command line arguments
    if (argc >= 2)
    {
        serverType = argv[1];

        if (serverType == "help" || serverType == "--help" || serverType == "-h")
        {
            printUsage(argv[0]);
            return 0;
        }
    }

    if (argc >= 3)
    {
        address = argv[2];
    }
    else
    {
        address = getDefaultAddress(serverType);
    }

    try
    {
        // Create the appropriate server strategy
        auto server = createServerStrategy(serverType);

        std::cout << "========================================" << std::endl;
        std::cout << "  C++ MCP Bridge Demo" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Server Type: " << server->getServerType() << std::endl;
        std::cout << "Address: " << address << std::endl;
        std::cout << "========================================" << std::endl;

        // Start the server (this will block)
        server->start(address);
    }
    catch (const std::invalid_argument& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
