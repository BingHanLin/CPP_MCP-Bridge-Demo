#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socket_t = SOCKET;
#define CLOSE_SOCKET closesocket
#define SOCKET_ERROR_VALUE SOCKET_ERROR
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using socket_t = int;
#define CLOSE_SOCKET close
#define SOCKET_ERROR_VALUE -1
#endif

#include "commandServer.hpp"

commandServer::commandServer(int port) : port_(port), running_(false)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

commandServer::~commandServer()
{
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool commandServer::start()
{
    if (running_)
    {
        return true;
    }

    running_ = true;
    server_thread_ = std::thread(&commandServer::serverLoop, this);

    std::cout << "Command Server started on port " << port_ << std::endl;
    return true;
}

void commandServer::stop()
{
    if (!running_)
    {
        return;
    }

    running_ = false;

    if (server_thread_.joinable())
    {
        server_thread_.join();
    }

    std::cout << "Command Server stopped" << std::endl;
}

bool commandServer::isRunning() const
{
    return running_;
}

void commandServer::registercommandHandler(const std::string &command,
                                           std::function<nlohmann::json(const nlohmann::json &)> handler)
{
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    commandServers_[command] = handler;
}

void commandServer::serverLoop()
{
    socket_t server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == SOCKET_ERROR_VALUE)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }

    // Set socket options
    int opt = 1;
#ifdef _WIN32
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
#else
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
#endif
        std::cerr << "Failed to set socket options" << std::endl;
        CLOSE_SOCKET(server_socket);
        return;
    }

    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Failed to bind socket" << std::endl;
        CLOSE_SOCKET(server_socket);
        return;
    }

    if (listen(server_socket, 5) < 0)
    {
        std::cerr << "Failed to listen on socket" << std::endl;
        CLOSE_SOCKET(server_socket);
        return;
    }

    std::cout << "Server listening on port " << port_ << std::endl;

    while (running_)
    {
        sockaddr_in client_addr;
#ifdef _WIN32
        int client_len = sizeof(client_addr);
#else
        socklen_t client_len = sizeof(client_addr);
#endif

        socket_t client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_len);
        if (client_socket != SOCKET_ERROR_VALUE)
        {
            std::cout << "Client connected" << std::endl;
            std::thread client_thread(&commandServer::handleClient, this, client_socket);
            client_thread.detach();
        }
    }

    CLOSE_SOCKET(server_socket);
}

void commandServer::handleClient(socket_t client_socket)
{
    while (running_)
    {
        try
        {
            std::string message = receiveFullMessage(client_socket);
            if (message.empty())
            {
                break;
            }

            nlohmann::json command = nlohmann::json::parse(message);
            nlohmann::json response = processCommand(command);

            if (!sendResponse(client_socket, response))
            {
                break;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error handling client: " << e.what() << std::endl;
            nlohmann::json error_response = {{"status", "error"}, {"message", e.what()}};
            sendResponse(client_socket, error_response);
            break;
        }
    }

    CLOSE_SOCKET(client_socket);
    std::cout << "Client disconnected" << std::endl;
}

nlohmann::json commandServer::processCommand(const nlohmann::json &command)
{
    try
    {
        std::string command_type = command.value("type", "");
        nlohmann::json params = command.value("params", nlohmann::json::object());

        std::lock_guard<std::mutex> lock(handlers_mutex_);
        auto handler_it = commandServers_.find(command_type);

        if (handler_it != commandServers_.end())
        {
            nlohmann::json result = handler_it->second(params);
            return {{"status", "success"}, {"result", result}};
        }
        else
        {
            return {{"status", "error"}, {"message", "Unknown command: " + command_type}};
        }
    }
    catch (const std::exception &e)
    {
        return {{"status", "error"}, {"message", e.what()}};
    }
}

std::string commandServer::receiveFullMessage(socket_t socket)
{
    char buffer[8192];
    std::string message;

    while (true)
    {
        int bytes_received = recv(socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0)
        {
            return "";
        }

        buffer[bytes_received] = '\0';
        message += buffer;

        // Check if we have a complete JSON message
        try
        {
            nlohmann::json::parse(message);
            return message;
        }
        catch (const nlohmann::json::parse_error &)
        {
            // Continue receiving
            continue;
        }
    }
}

bool commandServer::sendResponse(socket_t socket, const nlohmann::json &response)
{
    std::string response_str = response.dump();

    int bytes_sent = send(socket, response_str.c_str(), response_str.length(), 0);
    return bytes_sent == static_cast<int>(response_str.length());
}
