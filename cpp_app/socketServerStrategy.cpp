#include "socketServerStrategy.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>

#ifdef _WIN32
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

socketServerStrategy::socketServerStrategy(const int& port) : port_(port), running_(false)
{
#ifdef _WIN32
    initializeWinsock();
#endif
    registerHandlers();
}

socketServerStrategy::~socketServerStrategy()
{
    if (running_)
    {
        running_ = false;
        if (server_thread_.joinable())
        {
            server_thread_.join();
        }
    }
#ifdef _WIN32
    cleanupWinsock();
#endif
}

void socketServerStrategy::start()
{
    if (running_)
    {
        std::cout << "Socket server is already running" << std::endl;
        return;
    }

    try
    {
        running_ = true;

        std::cout << "Starting Socket Server on port " << port_ << "..." << std::endl;

        // Start server in a separate thread initially, then join to make it blocking
        server_thread_ = std::thread(&socketServerStrategy::serverLoop, this);
        server_thread_.join();  // Make it blocking like the original implementation
    }
    catch (const std::exception& e)
    {
        running_ = false;
        throw std::runtime_error("Failed to start socket server: " + std::string(e.what()));
    }
}

void socketServerStrategy::registerHandlers()
{
    // Helper lambda to register handlers more concisely
    auto registerHandler = [this](const std::string& command, auto memberFunc)
    {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        command_handlers_[command] = [this, memberFunc](const nlohmann::json& params)
        { return (handler_.*memberFunc)(params); };
    };

    // Register all command handlers
    registerHandler("get_software_info", &commandHandler::getSoftwareInfo);
    registerHandler("get_software_status", &commandHandler::getSoftwareStatus);
    registerHandler("create_object", &commandHandler::createObject);
    registerHandler("delete_object", &commandHandler::deleteObject);
    registerHandler("list_objects", &commandHandler::listObjects);
    registerHandler("get_object_info", &commandHandler::getObjectInfo);
    registerHandler("execute_software_command", &commandHandler::executeSoftwareCommand);
    registerHandler("save_project", &commandHandler::saveProject);
    registerHandler("load_project", &commandHandler::loadProject);
}

void socketServerStrategy::serverLoop()
{
    socket_t server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET)
    {
        throw std::runtime_error("Failed to create socket");
    }

    // Set socket options
    int opt = 1;
#ifdef _WIN32
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        closesocket(server_socket);
        throw std::runtime_error("Failed to bind socket to port " + std::to_string(port_));
    }

    if (listen(server_socket, 5) == SOCKET_ERROR)
    {
        closesocket(server_socket);
        throw std::runtime_error("Failed to listen on socket");
    }

    std::cout << "Socket server listening on port " << port_ << std::endl;

    while (running_)
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        socket_t client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);

        if (client_socket != INVALID_SOCKET)
        {
            std::thread(&socketServerStrategy::handleClient, this, client_socket).detach();
        }
        else if (running_)
        {
            std::cerr << "Failed to accept client connection" << std::endl;
        }
    }

    closesocket(server_socket);
}

void socketServerStrategy::handleClient(socket_t client_socket)
{
    try
    {
        char buffer[4096];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            std::string request_str(buffer);

            try
            {
                nlohmann::json request = nlohmann::json::parse(request_str);
                nlohmann::json response = processCommand(request);

                std::string response_str = response.dump();
                send(client_socket, response_str.c_str(), static_cast<int>(response_str.length()), 0);
            }
            catch (const std::exception& e)
            {
                nlohmann::json error_response = {{"error", "Invalid JSON or processing error"}, {"message", e.what()}};
                std::string response_str = error_response.dump();
                send(client_socket, response_str.c_str(), static_cast<int>(response_str.length()), 0);
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error handling client: " << e.what() << std::endl;
    }

    closesocket(client_socket);
}

nlohmann::json socketServerStrategy::processCommand(const nlohmann::json& request)
{
    if (!request.contains("command"))
    {
        return {{"error", "Missing 'command' field"}};
    }

    std::string command = request["command"];
    nlohmann::json params = request.value("params", nlohmann::json::object());

    std::lock_guard<std::mutex> lock(handlers_mutex_);
    auto it = command_handlers_.find(command);
    if (it != command_handlers_.end())
    {
        try
        {
            return it->second(params);
        }
        catch (const std::exception& e)
        {
            return {{"error", "Command execution failed"}, {"message", e.what()}};
        }
    }
    else
    {
        return {{"error", "Unknown command"}, {"command", command}};
    }
}

#ifdef _WIN32
void socketServerStrategy::initializeWinsock()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        throw std::runtime_error("WSAStartup failed: " + std::to_string(result));
    }
}

void socketServerStrategy::cleanupWinsock()
{
    WSACleanup();
}
#endif
