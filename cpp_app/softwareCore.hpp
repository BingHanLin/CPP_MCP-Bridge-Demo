#pragma once

#include <map>
#include <string>
#include <vector>

// Core business logic for the software
class softwareCore
{
  public:
    struct softwareInfo
    {
        std::string name_;
        std::string version_;
        bool isRunning_;
        std::string currentProject_;
        size_t totalObjects_;
    };

    // Object representation
    struct softwareObject
    {
        std::string name_;
        std::string type_;
        std::map<std::string, std::string> properties_;
    };

    softwareCore();  // Software information

    // Core operations
    softwareInfo getSoftwareInfo() const;
    softwareInfo getSoftwareStatus() const;

    // Object management
    std::string createObject(const std::string& name, const std::string& type,
                             const std::map<std::string, std::string>& properties = {});
    bool deleteObject(const std::string& objectId);
    std::vector<std::pair<std::string, softwareObject>> listObjects() const;
    bool getObjectInfo(const std::string& objectId, softwareObject& outObject) const;

    // Project management
    bool saveProject(const std::string& filename);
    bool loadProject(const std::string& filename);

    // Software operations
    bool executeCommand(const std::string& command, const std::map<std::string, std::string>& params = {});

  private:
    std::map<std::string, softwareObject> objects_;
    std::string currentProject_;
    bool isRunning_;
    std::string softwareName_;
    std::string version_;

    // Helper methods
    static std::string generateObjectId();
    static bool validateObjectType(const std::string& type);
    void initializeDefaultObjects();
};
