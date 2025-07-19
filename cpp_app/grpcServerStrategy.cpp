#include <iostream>
#include <stdexcept>

#include <grpcpp/grpcpp.h>

#include "grpcServerStrategy.hpp"

grpcServerStrategy::grpcServerStrategy()
{
    service_ = std::make_unique<grpcMcpService>(handler_);
}

grpcServerStrategy::~grpcServerStrategy()
{
    if (server_)
    {
        server_->Shutdown();
    }
}

void grpcServerStrategy::start(const std::string& address)
{
    try
    {
        grpc::ServerBuilder builder;

        // Listen on the given address without any authentication mechanism
        builder.AddListeningPort(address, grpc::InsecureServerCredentials());

        // Register our service
        builder.RegisterService(service_.get());

        // Finally assemble the server
        server_ = builder.BuildAndStart();

        if (!server_)
        {
            throw std::runtime_error("Failed to start gRPC server");
        }

        std::cout << "Starting gRPC Server on " << address << "..." << std::endl;

        // Wait for the server to shutdown (blocking)
        server_->Wait();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Failed to start gRPC server: " + std::string(e.what()));
    }
}

std::string grpcServerStrategy::getServerType() const
{
    return "gRPC";
}
