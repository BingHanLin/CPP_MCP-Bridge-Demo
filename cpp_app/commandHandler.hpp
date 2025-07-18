#pragma once

#include <map>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

class commandHandler
{
  public:
    commandHandler();

    // Basic software information
    nlohmann::json getSoftwareInfo(const nlohmann::json &params);
    nlohmann::json getSoftwareStatus(const nlohmann::json &params);

    // Object management
    nlohmann::json createObject(const nlohmann::json &params);
    nlohmann::json deleteObject(const nlohmann::json &params);
    nlohmann::json listObjects(const nlohmann::json &params);
    nlohmann::json getObjectInfo(const nlohmann::json &params);

    // Software operations
    nlohmann::json executeSoftwareCommand(const nlohmann::json &params);
    nlohmann::json saveProject(const nlohmann::json &params);
    nlohmann::json loadProject(const nlohmann::json &params);

  private:
    // Simulate software state
    struct softwareObject
    {
        std::string name;
        std::string type;
        std::map<std::string, std::string> properties;
    };

    std::map<std::string, softwareObject> objects_;
    std::string current_project_;
    bool is_running_;

    // Helper methods
    nlohmann::json objectToJson(const softwareObject &obj);
    std::string generateObjectId();
};
