#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
using socket_t = SOCKET;
#else
using socket_t = int;
#endif

#include "nlohmann/json.hpp"

class commandServer
{
  public:
    explicit commandServer(int port = 9876);
    ~commandServer();

    bool start();
    void stop();
    bool isRunning() const;

    // Command handler registration
    void registercommandHandler(const std::string &command,
                                std::function<nlohmann::json(const nlohmann::json &)> handler);

  private:
    int port_;
    std::atomic<bool> running_;
    std::thread server_thread_;
    std::mutex handlers_mutex_;
    std::map<std::string, std::function<nlohmann::json(const nlohmann::json &)>> commandServers_;

    void serverLoop();
    void handleClient(socket_t client_socket);
    nlohmann::json processCommand(const nlohmann::json &command);
    std::string receiveFullMessage(socket_t socket);
    bool sendResponse(socket_t socket, const nlohmann::json &response);
};
