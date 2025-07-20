#include "softwareCore.hpp"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>

softwareCore::softwareCore()
    : currentProject_("untitled_project"), isRunning_(true), softwareName_("My Example Software"), version_("1.0.0")
{
    initializeDefaultObjects();
}

softwareCore::softwareInfo softwareCore::getSoftwareInfo() const
{
    return {softwareName_, version_, isRunning_, currentProject_, objects_.size()};
}

softwareCore::softwareInfo softwareCore::getSoftwareStatus() const
{
    return getSoftwareInfo();  // Same for now, could be different
}

std::string softwareCore::createObject(const std::string& name, const std::string& type,
                                       const std::map<std::string, std::string>& properties)
{
    if (!validateObjectType(type))
    {
        return "";  // Invalid type
    }

    std::string id = generateObjectId();
    softwareObject obj;
    obj.name_ = name;
    obj.type_ = type;
    obj.properties_ = properties;
    obj.properties_["created_at"] = "now";
    obj.properties_["id"] = id;

    // Add type-specific default properties
    if (type == "cube" && obj.properties_.find("size") == obj.properties_.end())
    {
        obj.properties_["size"] = "1.0";
    }
    if (type == "sphere" && obj.properties_.find("radius") == obj.properties_.end())
    {
        obj.properties_["radius"] = "0.5";
    }
    if ((type == "cube" || type == "sphere") && obj.properties_.find("color") == obj.properties_.end())
    {
        obj.properties_["color"] = "white";
    }
    if (type == "camera")
    {
        if (obj.properties_.find("position") == obj.properties_.end())
        {
            obj.properties_["position"] = "0,0,5";
        }
        if (obj.properties_.find("rotation") == obj.properties_.end())
        {
            obj.properties_["rotation"] = "0,0,0";
        }
    }

    objects_[id] = obj;
    return id;
}

bool softwareCore::deleteObject(const std::string& objectId)
{
    auto it = objects_.find(objectId);
    if (it != objects_.end())
    {
        objects_.erase(it);
        return true;
    }
    return false;
}

std::vector<std::pair<std::string, softwareCore::softwareObject>> softwareCore::listObjects() const
{
    std::vector<std::pair<std::string, softwareObject>> result(objects_.size());
    for (const auto& pair : objects_)
    {
        result.emplace_back(pair.first, pair.second);
    }
    return result;
}

bool softwareCore::getObjectInfo(const std::string& objectId, softwareObject& outObject) const
{
    auto it = objects_.find(objectId);
    if (it != objects_.end())
    {
        outObject = it->second;
        return true;
    }
    return false;
}

bool softwareCore::saveProject(const std::string& filename)
{
    try
    {
        nlohmann::json project_data = {{"project_name", currentProject_}, {"objects", nlohmann::json::object()}};

        for (const auto& pair : objects_)
        {
            nlohmann::json obj_json = {
                {"name", pair.second.name_}, {"type", pair.second.type_}, {"properties", nlohmann::json::object()}};

            for (const auto& prop : pair.second.properties_)
            {
                obj_json["properties"][prop.first] = prop.second;
            }

            project_data["objects"][pair.first] = obj_json;
        }

        std::ofstream file(filename);
        if (file.is_open())
        {
            file << project_data.dump(4);
            file.close();
            return true;
        }
    }
    catch (const std::exception&)
    {
        // Handle error
    }
    return false;
}

bool softwareCore::loadProject(const std::string& filename)
{
    try
    {
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
                for (const auto& item : project_data["objects"].items())
                {
                    const std::string& id = item.key();
                    const auto& obj_data = item.value();
                    softwareObject obj;
                    obj.name_ = obj_data.value("name", "");
                    obj.type_ = obj_data.value("type", "");

                    if (obj_data.contains("properties"))
                    {
                        for (const auto& prop_item : obj_data["properties"].items())
                        {
                            obj.properties_[prop_item.key()] = prop_item.value().get<std::string>();
                        }
                    }

                    objects_[id] = obj;
                }
            }

            // Update current project name
            if (project_data.contains("project_name"))
            {
                currentProject_ = project_data["project_name"];
            }

            return true;
        }
    }
    catch (const std::exception&)
    {
        // Handle error
    }
    return false;
}

bool softwareCore::executeCommand(const std::string& command, const std::map<std::string, std::string>& params)
{
    if (command == "render")
    {
        // Simulate rendering
        return true;
    }
    else if (command == "clear_scene")
    {
        objects_.clear();
        return true;
    }
    else if (command == "reset_camera")
    {
        // Find and reset camera
        for (auto& pair : objects_)
        {
            if (pair.second.type_ == "camera")
            {
                pair.second.properties_["position"] = "0,0,5";
                pair.second.properties_["rotation"] = "0,0,0";
            }
        }
        return true;
    }
    return false;  // Unknown command
}

std::string softwareCore::generateObjectId()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);

    std::stringstream ss;
    ss << "obj_" << std::setfill('0') << std::setw(3) << dis(gen);
    return ss.str();
}

bool softwareCore::validateObjectType(const std::string& type)
{
    return type == "cube" || type == "sphere" || type == "camera";
}

void softwareCore::initializeDefaultObjects()
{
    // Initialize with some sample objects
    softwareObject obj1;
    obj1.name_ = "default_cube";
    obj1.type_ = "cube";
    obj1.properties_["size"] = "1.0";
    obj1.properties_["color"] = "white";

    softwareObject obj2;
    obj2.name_ = "default_camera";
    obj2.type_ = "camera";
    obj2.properties_["position"] = "0,0,5";
    obj2.properties_["rotation"] = "0,0,0";

    objects_["obj_001"] = obj1;
    objects_["obj_002"] = obj2;
}
