#include <chrono>
#include <iostream>
#include <thread>

#include "commandHandler.hpp"
#include "commandServer.hpp"

int main()
{
    std::cout << "Starting My Software with MCP integration..." << std::endl;

    // Create command handler
    commandHandler handler;

    // Create Command Server
    commandServer server(9876);

    // Register command handlers
    server.registercommandHandler("get_software_info",
                                  [&handler](const nlohmann::json &params) { return handler.getSoftwareInfo(params); });

    server.registercommandHandler(
        "get_software_status", [&handler](const nlohmann::json &params) { return handler.getSoftwareStatus(params); });

    server.registercommandHandler("create_object",
                                  [&handler](const nlohmann::json &params) { return handler.createObject(params); });

    server.registercommandHandler("delete_object",
                                  [&handler](const nlohmann::json &params) { return handler.deleteObject(params); });

    server.registercommandHandler("list_objects",
                                  [&handler](const nlohmann::json &params) { return handler.listObjects(params); });

    server.registercommandHandler("get_object_info",
                                  [&handler](const nlohmann::json &params) { return handler.getObjectInfo(params); });

    server.registercommandHandler("execute_software_command", [&handler](const nlohmann::json &params)
                                  { return handler.executeSoftwareCommand(params); });

    server.registercommandHandler("save_project",
                                  [&handler](const nlohmann::json &params) { return handler.saveProject(params); });

    server.registercommandHandler("load_project",
                                  [&handler](const nlohmann::json &params) { return handler.loadProject(params); });

    // Start the server
    if (server.start())
    {
        std::cout << "Command Server started successfully!" << std::endl;
        std::cout << "Press Ctrl+C to stop the server." << std::endl;

        // Keep the main thread alive
        while (server.isRunning())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    else
    {
        std::cerr << "Failed to start Command Server" << std::endl;
        return 1;
    }

    return 0;
}
