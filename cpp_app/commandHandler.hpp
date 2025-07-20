#pragma once

#include <map>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"
#include "softwareCore.hpp"

// Command handler - responsible for parsing commands and delegating to core
class commandHandler
{
  public:
    commandHandler();

    // Command processing - these methods parse JSON and delegate to core
    nlohmann::json getSoftwareInfo(const nlohmann::json &params);
    nlohmann::json getSoftwareStatus(const nlohmann::json &params);
    nlohmann::json createObject(const nlohmann::json &params);
    nlohmann::json deleteObject(const nlohmann::json &params);
    nlohmann::json listObjects(const nlohmann::json &params);
    nlohmann::json getObjectInfo(const nlohmann::json &params);
    nlohmann::json executeSoftwareCommand(const nlohmann::json &params);
    nlohmann::json saveProject(const nlohmann::json &params);
    nlohmann::json loadProject(const nlohmann::json &params);

  private:
    softwareCore core_;  // The actual business logic

    // Helper methods for JSON conversion
    static nlohmann::json objectToJson(const softwareCore::softwareObject &obj);
    static nlohmann::json softwareInfoToJson(const softwareCore::softwareInfo &info);
    static nlohmann::json createSuccessResponse(const nlohmann::json &data);
    static nlohmann::json createErrorResponse(const std::string &message);
};
