#include "commandHandler.hpp"

commandHandler::commandHandler() = default;

nlohmann::json commandHandler::getSoftwareInfo(const nlohmann::json &params)
{
    auto info = core_.getSoftwareInfo();
    return softwareInfoToJson(info);
}

nlohmann::json commandHandler::getSoftwareStatus(const nlohmann::json &params)
{
    auto status = core_.getSoftwareStatus();
    nlohmann::json result = {{"running", status.isRunning_},
                             {"current_project", status.currentProject_},
                             {"object_count", status.totalObjects_},
                             {"memory_usage", "45.2 MB"},
                             {"uptime", "2h 15m 30s"}};
    return result;
}

nlohmann::json commandHandler::createObject(const nlohmann::json &params)
{
    try
    {
        std::string name = params.value("name", "new_object");
        std::string type = params.value("type", "cube");

        // Extract properties from params
        std::map<std::string, std::string> properties;
        if (params.contains("size"))
        {
            properties["size"] = params["size"];
        }
        if (params.contains("radius"))
        {
            properties["radius"] = params["radius"];
        }
        if (params.contains("color"))
        {
            properties["color"] = params["color"];
        }
        if (params.contains("position"))
        {
            properties["position"] = params["position"];
        }
        if (params.contains("rotation"))
        {
            properties["rotation"] = params["rotation"];
        }

        std::string id = core_.createObject(name, type, properties);
        if (!id.empty())
        {
            softwareCore::softwareObject obj;
            if (core_.getObjectInfo(id, obj))
            {
                return createSuccessResponse({{"object_id", id}, {"object", objectToJson(obj)}});
            }
        }

        return createErrorResponse("Failed to create object");
    }
    catch (const std::exception &e)
    {
        return createErrorResponse(e.what());
    }
}

nlohmann::json commandHandler::deleteObject(const nlohmann::json &params)
{
    try
    {
        std::string id = params.value("id", "");

        if (core_.deleteObject(id))
        {
            return createSuccessResponse({{"message", "Object deleted successfully"}});
        }
        else
        {
            return createErrorResponse("Object not found");
        }
    }
    catch (const std::exception &e)
    {
        return createErrorResponse(e.what());
    }
}

nlohmann::json commandHandler::listObjects(const nlohmann::json &params)
{
    auto objects = core_.listObjects();
    nlohmann::json objects_list = nlohmann::json::array();

    for (const auto &pair : objects)
    {
        nlohmann::json obj_summary = {{"id", pair.first}, {"name", pair.second.name_}, {"type", pair.second.type_}};
        objects_list.push_back(obj_summary);
    }

    return {{"total_count", objects.size()}, {"objects", objects_list}};
}

nlohmann::json commandHandler::getObjectInfo(const nlohmann::json &params)
{
    try
    {
        std::string id = params.value("id", "");
        softwareCore::softwareObject obj;

        if (core_.getObjectInfo(id, obj))
        {
            return createSuccessResponse({{"object", objectToJson(obj)}});
        }
        else
        {
            return createErrorResponse("Object not found");
        }
    }
    catch (const std::exception &e)
    {
        return createErrorResponse(e.what());
    }
}

nlohmann::json commandHandler::executeSoftwareCommand(const nlohmann::json &params)
{
    try
    {
        std::string command = params.value("command", "");
        std::map<std::string, std::string> cmdParams;

        // Extract any additional parameters
        if (params.contains("params") && params["params"].is_object())
        {
            for (const auto &item : params["params"].items())
            {
                if (item.value().is_string())
                {
                    cmdParams[item.key()] = item.value().get<std::string>();
                }
            }
        }

        if (core_.executeCommand(command, cmdParams))
        {
            if (command == "render")
            {
                return createSuccessResponse(
                    {{"message", "Render completed successfully"}, {"output_file", "render_output.png"}});
            }
            else if (command == "clear_scene")
            {
                return createSuccessResponse({{"message", "Scene cleared successfully"}});
            }
            else if (command == "reset_camera")
            {
                return createSuccessResponse({{"message", "Camera reset successfully"}});
            }
            else
            {
                return createSuccessResponse({{"message", "Command executed successfully"}});
            }
        }
        else
        {
            return createErrorResponse("Unknown command: " + command);
        }
    }
    catch (const std::exception &e)
    {
        return createErrorResponse(e.what());
    }
}

nlohmann::json commandHandler::saveProject(const nlohmann::json &params)
{
    try
    {
        auto info = core_.getSoftwareInfo();
        std::string filename = params.value("filename", info.currentProject_ + ".json");

        if (core_.saveProject(filename))
        {
            return createSuccessResponse({{"message", "Project saved successfully"}, {"filename", filename}});
        }
        else
        {
            return createErrorResponse("Could not save project to file: " + filename);
        }
    }
    catch (const std::exception &e)
    {
        return createErrorResponse(e.what());
    }
}

nlohmann::json commandHandler::loadProject(const nlohmann::json &params)
{
    try
    {
        std::string filename = params.value("filename", "");

        if (core_.loadProject(filename))
        {
            auto objects = core_.listObjects();
            return createSuccessResponse({{"message", "Project loaded successfully"},
                                          {"filename", filename},
                                          {"objects_loaded", objects.size()}});
        }
        else
        {
            return createErrorResponse("Could not load project from file: " + filename);
        }
    }
    catch (const std::exception &e)
    {
        return createErrorResponse(e.what());
    }
}

nlohmann::json commandHandler::objectToJson(const softwareCore::softwareObject &obj)
{
    nlohmann::json properties = nlohmann::json::object();
    for (const auto &prop : obj.properties_)
    {
        properties[prop.first] = prop.second;
    }

    return {{"name", obj.name_}, {"type", obj.type_}, {"properties", properties}};
}

nlohmann::json commandHandler::softwareInfoToJson(const softwareCore::softwareInfo &info)
{
    return {
        {"software_name", info.name_},
        {"version", info.version_},
        {"status", info.isRunning_ ? "running" : "stopped"},
        {"current_project", info.currentProject_},
        {"total_objects", info.totalObjects_},
        {"available_commands", nlohmann::json::array({"get_software_info", "get_software_status", "create_object",
                                                      "delete_object", "list_objects", "get_object_info",
                                                      "execute_software_command", "save_project", "load_project"})}};
}

nlohmann::json commandHandler::createSuccessResponse(const nlohmann::json &data)
{
    nlohmann::json result = {{"success", true}};
    for (const auto &item : data.items())
    {
        result[item.key()] = item.value();
    }
    return result;
}

nlohmann::json commandHandler::createErrorResponse(const std::string &message)
{
    return {{"success", false}, {"error", message}};
}
