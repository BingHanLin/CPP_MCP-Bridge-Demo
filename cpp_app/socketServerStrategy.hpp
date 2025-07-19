#pragma once

#include "nlohmann/json.hpp"
#include "serverStrategy.hpp"
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

// Socket server strategy implementation that directly implements the socket server
class socketServerStrategy : public serverStrategy
{
  public:
    socketServerStrategy();
    ~socketServerStrategy() override;

    void start(const std::string& address) override;

  private:
    int port_;
    std::atomic<bool> running_;
    std::thread server_thread_;
    std::mutex handlers_mutex_;
    std::map<std::string, std::function<nlohmann::json(const nlohmann::json&)>> command_handlers_;

    void registerHandlers();
    void serverLoop();
    void handleClient(socket_t client_socket);
    nlohmann::json processCommand(const nlohmann::json& request);
    static int parsePort(const std::string& address);

#ifdef _WIN32
    void initializeWinsock();
    void cleanupWinsock();
#endif
};
