#include "grpcServerStrategy.hpp"
#include "nlohmann/json.hpp"
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <stdexcept>

grpcServerStrategy::grpcServerStrategy()
{
    // No need to create a separate service - this class IS the service
}

grpcServerStrategy::~grpcServerStrategy()
{
    if (server_)
    {
        server_->Shutdown();
    }
}

void grpcServerStrategy::start(const std::string& address)
{
    try
    {
        grpc::ServerBuilder builder;

        // Listen on the given address without any authentication mechanism
        builder.AddListeningPort(address, grpc::InsecureServerCredentials());

        // Register this object as the service (since we inherit from MCPService::Service)
        builder.RegisterService(this);

        // Finally assemble the server
        server_ = builder.BuildAndStart();

        if (!server_)
        {
            throw std::runtime_error("Failed to start gRPC server");
        }

        std::cout << "Starting gRPC Server on " << address << "..." << std::endl;

        // Wait for the server to shutdown (blocking)
        server_->Wait();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Failed to start gRPC server: " + std::string(e.what()));
    }
}

std::string grpcServerStrategy::getServerType() const
{
    return "gRPC";
}

// gRPC service method implementations
grpc::Status grpcServerStrategy::GetSoftwareInfo(grpc::ServerContext* context,
                                                 const mcp::GetSoftwareInfoRequest* request,
                                                 mcp::GetSoftwareInfoResponse* response)
{
    try
    {
        nlohmann::json params = nlohmann::json::object();
        nlohmann::json result = handler_.getSoftwareInfo(params);

        auto* info = response->mutable_info();
        jsonToProto(result, info);

        return grpc::Status::OK;
    }
    catch (const std::exception& e)
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status grpcServerStrategy::GetSoftwareStatus(grpc::ServerContext* context,
                                                   const mcp::GetSoftwareStatusRequest* request,
                                                   mcp::GetSoftwareStatusResponse* response)
{
    try
    {
        nlohmann::json params = nlohmann::json::object();
        nlohmann::json result = handler_.getSoftwareStatus(params);

        auto* status = response->mutable_status();
        jsonToProto(result, status);

        return grpc::Status::OK;
    }
    catch (const std::exception& e)
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status grpcServerStrategy::CreateObject(grpc::ServerContext* context, const mcp::CreateObjectRequest* request,
                                              mcp::CreateObjectResponse* response)
{
    try
    {
        nlohmann::json params = protoToJson(*request);
        nlohmann::json result = handler_.createObject(params);

        jsonToProto(result, response);

        return grpc::Status::OK;
    }
    catch (const std::exception& e)
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status grpcServerStrategy::DeleteObject(grpc::ServerContext* context, const mcp::DeleteObjectRequest* request,
                                              mcp::DeleteObjectResponse* response)
{
    try
    {
        nlohmann::json params = protoToJson(*request);
        nlohmann::json result = handler_.deleteObject(params);

        jsonToProto(result, response);

        return grpc::Status::OK;
    }
    catch (const std::exception& e)
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status grpcServerStrategy::ListObjects(grpc::ServerContext* context, const mcp::ListObjectsRequest* request,
                                             mcp::ListObjectsResponse* response)
{
    try
    {
        nlohmann::json params = nlohmann::json::object();
        nlohmann::json result = handler_.listObjects(params);

        jsonToProto(result, response);

        return grpc::Status::OK;
    }
    catch (const std::exception& e)
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status grpcServerStrategy::GetObjectInfo(grpc::ServerContext* context, const mcp::GetObjectInfoRequest* request,
                                               mcp::GetObjectInfoResponse* response)
{
    try
    {
        nlohmann::json params = protoToJson(*request);
        nlohmann::json result = handler_.getObjectInfo(params);

        jsonToProto(result, response);

        return grpc::Status::OK;
    }
    catch (const std::exception& e)
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status grpcServerStrategy::ExecuteSoftwareCommand(grpc::ServerContext* context,
                                                        const mcp::ExecuteSoftwareCommandRequest* request,
                                                        mcp::ExecuteSoftwareCommandResponse* response)
{
    try
    {
        nlohmann::json params = protoToJson(*request);
        nlohmann::json result = handler_.executeSoftwareCommand(params);

        jsonToProto(result, response);

        return grpc::Status::OK;
    }
    catch (const std::exception& e)
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status grpcServerStrategy::SaveProject(grpc::ServerContext* context, const mcp::SaveProjectRequest* request,
                                             mcp::SaveProjectResponse* response)
{
    try
    {
        nlohmann::json params = protoToJson(*request);
        nlohmann::json result = handler_.saveProject(params);

        jsonToProto(result, response);

        return grpc::Status::OK;
    }
    catch (const std::exception& e)
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status grpcServerStrategy::LoadProject(grpc::ServerContext* context, const mcp::LoadProjectRequest* request,
                                             mcp::LoadProjectResponse* response)
{
    try
    {
        nlohmann::json params = protoToJson(*request);
        nlohmann::json result = handler_.loadProject(params);

        jsonToProto(result, response);

        return grpc::Status::OK;
    }
    catch (const std::exception& e)
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

// Helper methods for conversion between protobuf and JSON
nlohmann::json grpcServerStrategy::protoToJson(const mcp::CreateObjectRequest& request)
{
    nlohmann::json json;
    json["name"] = request.name();
    json["object_type"] = request.type();  // 'type' in protobuf, 'object_type' in JSON

    // Convert properties to kwargs-style
    nlohmann::json kwargs;
    for (const auto& prop : request.properties())
    {
        kwargs[prop.key()] = prop.value();
    }
    json["kwargs"] = kwargs.dump();

    return json;
}

nlohmann::json grpcServerStrategy::protoToJson(const mcp::DeleteObjectRequest& request)
{
    nlohmann::json json;
    json["object_id"] = request.object_id();
    return json;
}

nlohmann::json grpcServerStrategy::protoToJson(const mcp::GetObjectInfoRequest& request)
{
    nlohmann::json json;
    json["object_id"] = request.object_id();
    return json;
}

nlohmann::json grpcServerStrategy::protoToJson(const mcp::ExecuteSoftwareCommandRequest& request)
{
    nlohmann::json json;
    json["command"] = request.command();

    // Convert params to kwargs-style
    nlohmann::json kwargs;
    for (const auto& param : request.params())
    {
        kwargs[param.key()] = param.value();
    }
    json["kwargs"] = kwargs.dump();

    return json;
}

nlohmann::json grpcServerStrategy::protoToJson(const mcp::SaveProjectRequest& request)
{
    nlohmann::json json;
    if (!request.filename().empty())
    {
        json["filename"] = request.filename();
    }
    return json;
}

nlohmann::json grpcServerStrategy::protoToJson(const mcp::LoadProjectRequest& request)
{
    nlohmann::json json;
    json["filename"] = request.filename();
    return json;
}

void grpcServerStrategy::jsonToProto(const nlohmann::json& json, mcp::SoftwareInfo* info)
{
    if (json.contains("name")) info->set_software_name(json["name"].get<std::string>());
    if (json.contains("version")) info->set_version(json["version"].get<std::string>());
    if (json.contains("status")) info->set_status(json["status"].get<std::string>());
    if (json.contains("current_project")) info->set_current_project(json["current_project"].get<std::string>());
    if (json.contains("total_objects")) info->set_total_objects(json["total_objects"].get<int32_t>());
    if (json.contains("available_commands") && json["available_commands"].is_array())
    {
        for (const auto& cmd : json["available_commands"])
        {
            info->add_available_commands(cmd.get<std::string>());
        }
    }
}

void grpcServerStrategy::jsonToProto(const nlohmann::json& json, mcp::SoftwareStatus* status)
{
    if (json.contains("running")) status->set_running(json["running"].get<bool>());
    if (json.contains("current_project")) status->set_current_project(json["current_project"].get<std::string>());
    if (json.contains("object_count")) status->set_object_count(json["object_count"].get<int32_t>());
    if (json.contains("memory_usage")) status->set_memory_usage(json["memory_usage"].get<std::string>());
    if (json.contains("uptime")) status->set_uptime(json["uptime"].get<std::string>());
}

void grpcServerStrategy::jsonToProto(const nlohmann::json& json, mcp::CreateObjectResponse* response)
{
    if (json.contains("success")) response->set_success(json["success"].get<bool>());
    if (json.contains("object_id")) response->set_object_id(json["object_id"].get<std::string>());
    if (json.contains("error")) response->set_error(json["error"].get<std::string>());

    // Handle object details if present
    if (json.contains("object"))
    {
        auto* obj = response->mutable_object();
        const auto& object_info = json["object"];
        if (object_info.contains("name")) obj->set_name(object_info["name"].get<std::string>());
        if (object_info.contains("type")) obj->set_type(object_info["type"].get<std::string>());
    }
}

void grpcServerStrategy::jsonToProto(const nlohmann::json& json, mcp::DeleteObjectResponse* response)
{
    if (json.contains("success")) response->set_success(json["success"].get<bool>());
    if (json.contains("error")) response->set_error(json["error"].get<std::string>());
    if (json.contains("message")) response->set_message(json["message"].get<std::string>());
}

void grpcServerStrategy::jsonToProto(const nlohmann::json& json, mcp::ListObjectsResponse* response)
{
    if (json.contains("total_count")) response->set_total_count(json["total_count"].get<int32_t>());

    if (json.contains("objects") && json["objects"].is_array())
    {
        for (const auto& obj : json["objects"])
        {
            auto* object_summary = response->add_objects();
            if (obj.contains("id")) object_summary->set_id(obj["id"].get<std::string>());
            if (obj.contains("name")) object_summary->set_name(obj["name"].get<std::string>());
            if (obj.contains("type")) object_summary->set_type(obj["type"].get<std::string>());
        }
    }
}

void grpcServerStrategy::jsonToProto(const nlohmann::json& json, mcp::GetObjectInfoResponse* response)
{
    if (json.contains("success")) response->set_success(json["success"].get<bool>());
    if (json.contains("error")) response->set_error(json["error"].get<std::string>());

    if (json.contains("object"))
    {
        auto* obj = response->mutable_object();
        const auto& object_info = json["object"];
        if (object_info.contains("name")) obj->set_name(object_info["name"].get<std::string>());
        if (object_info.contains("type")) obj->set_type(object_info["type"].get<std::string>());
        // Handle properties if needed
        if (object_info.contains("properties") && object_info["properties"].is_object())
        {
            for (const auto& [key, value] : object_info["properties"].items())
            {
                auto* prop = obj->add_properties();
                prop->set_key(key);
                prop->set_value(value.is_string() ? value.get<std::string>() : value.dump());
            }
        }
    }
}

void grpcServerStrategy::jsonToProto(const nlohmann::json& json, mcp::ExecuteSoftwareCommandResponse* response)
{
    if (json.contains("success")) response->set_success(json["success"].get<bool>());
    if (json.contains("error")) response->set_error(json["error"].get<std::string>());
    if (json.contains("message")) response->set_message(json["message"].get<std::string>());
    if (json.contains("output_file")) response->set_output_file(json["output_file"].get<std::string>());
}

void grpcServerStrategy::jsonToProto(const nlohmann::json& json, mcp::SaveProjectResponse* response)
{
    if (json.contains("success")) response->set_success(json["success"].get<bool>());
    if (json.contains("error")) response->set_error(json["error"].get<std::string>());
    if (json.contains("message")) response->set_message(json["message"].get<std::string>());
    if (json.contains("filename")) response->set_filename(json["filename"].get<std::string>());
}

void grpcServerStrategy::jsonToProto(const nlohmann::json& json, mcp::LoadProjectResponse* response)
{
    if (json.contains("success")) response->set_success(json["success"].get<bool>());
    if (json.contains("error")) response->set_error(json["error"].get<std::string>());
    if (json.contains("message")) response->set_message(json["message"].get<std::string>());
    if (json.contains("filename")) response->set_filename(json["filename"].get<std::string>());
    if (json.contains("objects_loaded")) response->set_objects_loaded(json["objects_loaded"].get<int32_t>());
}
