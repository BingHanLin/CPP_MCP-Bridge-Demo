#pragma once

#include <memory>

#include "grpcpp/grpcpp.h"
#include "mcp_service.grpc.pb.h"
#include "serverStrategy.hpp"

// gRPC server strategy implementation that directly implements the gRPC service
class grpcServerStrategy : public serverStrategy, public mcp::MCPService::Service
{
  public:
    explicit grpcServerStrategy(const std::string& address);
    ~grpcServerStrategy() override;

    void start() override;

    // gRPC service method implementations
    grpc::Status GetSoftwareInfo(grpc::ServerContext* context, const mcp::GetSoftwareInfoRequest* request,
                                 mcp::GetSoftwareInfoResponse* response) override;

    grpc::Status GetSoftwareStatus(grpc::ServerContext* context, const mcp::GetSoftwareStatusRequest* request,
                                   mcp::GetSoftwareStatusResponse* response) override;

    grpc::Status CreateObject(grpc::ServerContext* context, const mcp::CreateObjectRequest* request,
                              mcp::CreateObjectResponse* response) override;

    grpc::Status DeleteObject(grpc::ServerContext* context, const mcp::DeleteObjectRequest* request,
                              mcp::DeleteObjectResponse* response) override;

    grpc::Status ListObjects(grpc::ServerContext* context, const mcp::ListObjectsRequest* request,
                             mcp::ListObjectsResponse* response) override;

    grpc::Status GetObjectInfo(grpc::ServerContext* context, const mcp::GetObjectInfoRequest* request,
                               mcp::GetObjectInfoResponse* response) override;

    grpc::Status ExecuteSoftwareCommand(grpc::ServerContext* context, const mcp::ExecuteSoftwareCommandRequest* request,
                                        mcp::ExecuteSoftwareCommandResponse* response) override;

    grpc::Status SaveProject(grpc::ServerContext* context, const mcp::SaveProjectRequest* request,
                             mcp::SaveProjectResponse* response) override;

    grpc::Status LoadProject(grpc::ServerContext* context, const mcp::LoadProjectRequest* request,
                             mcp::LoadProjectResponse* response) override;

  private:
    std::unique_ptr<grpc::Server> server_;
    std::string address_;

    // Helper methods for conversion between protobuf and JSON
    static nlohmann::json protoToJson(const mcp::CreateObjectRequest& request);
    static nlohmann::json protoToJson(const mcp::DeleteObjectRequest& request);
    static nlohmann::json protoToJson(const mcp::GetObjectInfoRequest& request);
    static nlohmann::json protoToJson(const mcp::ExecuteSoftwareCommandRequest& request);
    static nlohmann::json protoToJson(const mcp::SaveProjectRequest& request);
    static nlohmann::json protoToJson(const mcp::LoadProjectRequest& request);

    static void jsonToProto(const nlohmann::json& json, mcp::SoftwareInfo* info);
    static void jsonToProto(const nlohmann::json& json, mcp::SoftwareStatus* status);
    static void jsonToProto(const nlohmann::json& json, mcp::CreateObjectResponse* response);
    static void jsonToProto(const nlohmann::json& json, mcp::DeleteObjectResponse* response);
    static void jsonToProto(const nlohmann::json& json, mcp::ListObjectsResponse* response);
    static void jsonToProto(const nlohmann::json& json, mcp::GetObjectInfoResponse* response);
    static void jsonToProto(const nlohmann::json& json, mcp::ExecuteSoftwareCommandResponse* response);
    static void jsonToProto(const nlohmann::json& json, mcp::SaveProjectResponse* response);
    static void jsonToProto(const nlohmann::json& json, mcp::LoadProjectResponse* response);
};
