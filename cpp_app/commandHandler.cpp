#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>


#include "commandHandler.hpp"

commandHandler::commandHandler() : current_project_("untitled_project"), is_running_(true)
{
    // Initialize with some sample objects
    softwareObject obj1 = {"default_cube", "cube", {{"size", "1.0"}, {"color", "white"}}};
    softwareObject obj2 = {"default_camera", "camera", {{"position", "0,0,5"}, {"rotation", "0,0,0"}}};

    objects_["obj_001"] = obj1;
    objects_["obj_002"] = obj2;
}

nlohmann::json commandHandler::getSoftwareInfo(const nlohmann::json &params)
{
    nlohmann::json result = {
        {"software_name", "My Example Software"},
        {"version", "1.0.0"},
        {"status", is_running_ ? "running" : "stopped"},
        {"current_project", current_project_},
        {"total_objects", objects_.size()},
        {"available_commands", nlohmann::json::array({"get_software_info", "get_software_status", "create_object",
                                                      "delete_object", "list_objects", "get_object_info",
                                                      "execute_software_command", "save_project", "load_project"})}};

    return result;
}

nlohmann::json commandHandler::getSoftwareStatus(const nlohmann::json &params)
{
    nlohmann::json result = {{"running", is_running_},
                             {"current_project", current_project_},
                             {"object_count", objects_.size()},
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

        std::string id = generateObjectId();

        softwareObject obj;
        obj.name = name;
        obj.type = type;
        obj.properties["created_at"] = "now";
        obj.properties["id"] = id;

        // Add type-specific properties
        if (type == "cube")
        {
            obj.properties["size"] = params.value("size", "1.0");
            obj.properties["color"] = params.value("color", "white");
        }
        else if (type == "sphere")
        {
            obj.properties["radius"] = params.value("radius", "0.5");
            obj.properties["color"] = params.value("color", "white");
        }
        else if (type == "camera")
        {
            obj.properties["position"] = params.value("position", "0,0,5");
            obj.properties["rotation"] = params.value("rotation", "0,0,0");
        }

        objects_[id] = obj;

        return {{"success", true}, {"object_id", id}, {"object", objectToJson(obj)}};
    }
    catch (const std::exception &e)
    {
        return {{"success", false}, {"error", e.what()}};
    }
}

nlohmann::json commandHandler::deleteObject(const nlohmann::json &params)
{
    try
    {
        std::string id = params.value("id", "");

        auto it = objects_.find(id);
        if (it != objects_.end())
        {
            objects_.erase(it);
            return {{"success", true}, {"message", "Object deleted successfully"}};
        }
        else
        {
            return {{"success", false}, {"error", "Object not found"}};
        }
    }
    catch (const std::exception &e)
    {
        return {{"success", false}, {"error", e.what()}};
    }
}

nlohmann::json commandHandler::listObjects(const nlohmann::json &params)
{
    nlohmann::json objects_list = nlohmann::json::array();

    for (const auto &pair : objects_)
    {
        nlohmann::json obj_summary = {{"id", pair.first}, {"name", pair.second.name}, {"type", pair.second.type}};
        objects_list.push_back(obj_summary);
    }

    return {{"total_count", objects_.size()}, {"objects", objects_list}};
}

nlohmann::json commandHandler::getObjectInfo(const nlohmann::json &params)
{
    try
    {
        std::string id = params.value("id", "");

        auto it = objects_.find(id);
        if (it != objects_.end())
        {
            return {{"success", true}, {"object", objectToJson(it->second)}};
        }
        else
        {
            return {{"success", false}, {"error", "Object not found"}};
        }
    }
    catch (const std::exception &e)
    {
        return {{"success", false}, {"error", e.what()}};
    }
}

nlohmann::json commandHandler::executeSoftwareCommand(const nlohmann::json &params)
{
    try
    {
        std::string command = params.value("command", "");

        if (command == "render")
        {
            return {
                {"success", true}, {"message", "Render completed successfully"}, {"output_file", "render_output.png"}};
        }
        else if (command == "clear_scene")
        {
            objects_.clear();
            return {{"success", true}, {"message", "Scene cleared successfully"}};
        }
        else if (command == "reset_camera")
        {
            // Find and reset camera
            for (auto &pair : objects_)
            {
                if (pair.second.type == "camera")
                {
                    pair.second.properties["position"] = "0,0,5";
                    pair.second.properties["rotation"] = "0,0,0";
                }
            }
            return {{"success", true}, {"message", "Camera reset successfully"}};
        }
        else
        {
            return {{"success", false}, {"error", "Unknown command: " + command}};
        }
    }
    catch (const std::exception &e)
    {
        return {{"success", false}, {"error", e.what()}};
    }
}

nlohmann::json commandHandler::saveProject(const nlohmann::json &params)
{
    try
    {
        std::string filename = params.value("filename", current_project_ + ".json");

        // Create project data structure
        nlohmann::json project_data = {{"project_name", current_project_}, {"objects", nlohmann::json::object()}};

        for (const auto &pair : objects_)
        {
            project_data["objects"][pair.first] = objectToJson(pair.second);
        }

        // Actually write the file to disk
        std::ofstream file(filename);
        if (file.is_open())
        {
            file << project_data.dump(4);  // Pretty print with 4 spaces indentation
            file.close();
            return {{"success", true}, {"message", "Project saved successfully"}, {"filename", filename}};
        }
        else
        {
            return {{"success", false}, {"error", "Could not open file for writing: " + filename}};
        }
    }
    catch (const std::exception &e)
    {
        return {{"success", false}, {"error", e.what()}};
    }
}

nlohmann::json commandHandler::loadProject(const nlohmann::json &params)
{
    try
    {
        std::string filename = params.value("filename", "");

        // Actually read the file from disk
        std::ifstream file(filename);
        if (file.is_open())
        {
            nlohmann::json project_data;
            file >> project_data;
            file.close();

            // Clear existing objects
            objects_.clear();

            // Load objects from file
            if (project_data.contains("objects"))
            {
                for (const auto &[id, obj_data] : project_data["objects"].items())
                {
                    softwareObject obj;
                    obj.name = obj_data.value("name", "");
                    obj.type = obj_data.value("type", "");

                    if (obj_data.contains("properties"))
                    {
                        for (const auto &[prop_key, prop_value] : obj_data["properties"].items())
                        {
                            obj.properties[prop_key] = prop_value.get<std::string>();
                        }
                    }

                    objects_[id] = obj;
                }
            }

            // Update current project name
            if (project_data.contains("project_name"))
            {
                current_project_ = project_data["project_name"];
            }

            return {{"success", true},
                    {"message", "Project loaded successfully"},
                    {"filename", filename},
                    {"objects_loaded", objects_.size()}};
        }
        else
        {
            return {{"success", false}, {"error", "Could not open file for reading: " + filename}};
        }
    }
    catch (const std::exception &e)
    {
        return {{"success", false}, {"error", e.what()}};
    }
}

nlohmann::json commandHandler::objectToJson(const softwareObject &obj)
{
    nlohmann::json properties = nlohmann::json::object();
    for (const auto &prop : obj.properties)
    {
        properties[prop.first] = prop.second;
    }

    return {{"name", obj.name}, {"type", obj.type}, {"properties", properties}};
}

std::string commandHandler::generateObjectId()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);

    std::stringstream ss;
    ss << "obj_" << std::setfill('0') << std::setw(3) << dis(gen);
    return ss.str();
}
