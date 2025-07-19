#!/usr/bin/env python3
"""
Python gRPC client example for MCP Service
"""

import grpc
import sys
import os

# Add the generated protobuf files to the path
# You'll need to generate Python files from the proto:
# python -m grpc_tools.protoc -I../proto --python_out=. --grpc_python_out=. ../proto/mcp_service.proto

try:
    import mcp_service_pb2
    import mcp_service_pb2_grpc
except ImportError:
    print("Error: Please generate Python protobuf files first:")
    print("pip install grpcio-tools")
    print("python -m grpc_tools.protoc -I../proto --python_out=. --grpc_python_out=. ../proto/mcp_service.proto")
    sys.exit(1)


class MCPClient:
    def __init__(self, server_address='localhost:50051'):
        self.server_address = server_address
        self.channel = None
        self.stub = None

    def connect(self):
        """Connect to the gRPC server"""
        self.channel = grpc.insecure_channel(self.server_address)
        self.stub = mcp_service_pb2_grpc.MCPServiceStub(self.channel)
        print(f"Connected to gRPC server at {self.server_address}")

    def disconnect(self):
        """Disconnect from the gRPC server"""
        if self.channel:
            self.channel.close()
            print("Disconnected from gRPC server")

    def get_software_info(self):
        """Get software information"""
        try:
            request = mcp_service_pb2.GetSoftwareInfoRequest()
            response = self.stub.GetSoftwareInfo(request)
            print("Software Info:")
            print(f"  Name: {response.info.software_name}")
            print(f"  Version: {response.info.version}")
            print(f"  Status: {response.info.status}")
            print(f"  Current Project: {response.info.current_project}")
            print(f"  Total Objects: {response.info.total_objects}")
            print(
                f"  Available Commands: {list(response.info.available_commands)}")
            return response
        except grpc.RpcError as e:
            print(f"Error getting software info: {e}")
            return None

    def get_software_status(self):
        """Get software status"""
        try:
            request = mcp_service_pb2.GetSoftwareStatusRequest()
            response = self.stub.GetSoftwareStatus(request)
            print("Software Status:")
            print(f"  Running: {response.status.running}")
            print(f"  Current Project: {response.status.current_project}")
            print(f"  Object Count: {response.status.object_count}")
            print(f"  Memory Usage: {response.status.memory_usage}")
            print(f"  Uptime: {response.status.uptime}")
            return response
        except grpc.RpcError as e:
            print(f"Error getting software status: {e}")
            return None

    def create_object(self, name, obj_type, properties=None):
        """Create a new object"""
        try:
            request = mcp_service_pb2.CreateObjectRequest()
            request.name = name
            request.type = obj_type

            if properties:
                for key, value in properties.items():
                    prop = request.properties.add()
                    prop.key = key
                    prop.value = str(value)

            response = self.stub.CreateObject(request)

            if response.success:
                print(f"Object created successfully:")
                print(f"  ID: {response.object_id}")
                print(f"  Name: {response.object.name}")
                print(f"  Type: {response.object.type}")
                if response.object.properties:
                    print(f"  Properties:")
                    for prop in response.object.properties:
                        print(f"    {prop.key}: {prop.value}")
            else:
                print(f"Failed to create object: {response.error}")

            return response
        except grpc.RpcError as e:
            print(f"Error creating object: {e}")
            return None

    def list_objects(self):
        """List all objects"""
        try:
            request = mcp_service_pb2.ListObjectsRequest()
            response = self.stub.ListObjects(request)

            print(f"Objects ({response.total_count}):")
            for obj in response.objects:
                print(f"  {obj.id}: {obj.name} ({obj.type})")

            return response
        except grpc.RpcError as e:
            print(f"Error listing objects: {e}")
            return None

    def delete_object(self, object_id):
        """Delete an object"""
        try:
            request = mcp_service_pb2.DeleteObjectRequest()
            request.object_id = object_id

            response = self.stub.DeleteObject(request)

            if response.success:
                print(f"Object deleted successfully: {response.message}")
            else:
                print(f"Failed to delete object: {response.error}")

            return response
        except grpc.RpcError as e:
            print(f"Error deleting object: {e}")
            return None

    def execute_command(self, command, params=None):
        """Execute a software command"""
        try:
            request = mcp_service_pb2.ExecuteSoftwareCommandRequest()
            request.command = command

            if params:
                for key, value in params.items():
                    param = request.params.add()
                    param.key = key
                    param.value = str(value)

            response = self.stub.ExecuteSoftwareCommand(request)

            if response.success:
                print(f"Command executed successfully: {response.message}")
                if response.output_file:
                    print(f"Output file: {response.output_file}")
            else:
                print(f"Failed to execute command: {response.error}")

            return response
        except grpc.RpcError as e:
            print(f"Error executing command: {e}")
            return None


def main():
    """Example usage of the MCP gRPC client"""
    client = MCPClient()

    try:
        # Connect to server
        client.connect()

        # Get software info
        print("\n=== Software Info ===")
        client.get_software_info()

        # Get software status
        print("\n=== Software Status ===")
        client.get_software_status()

        # List existing objects
        print("\n=== List Objects ===")
        client.list_objects()

        # Create a new object
        print("\n=== Create Object ===")
        client.create_object("my_cube", "cube", {
                             "size": "2.0", "color": "red"})

        # List objects again
        print("\n=== List Objects After Creation ===")
        client.list_objects()

        # Execute a command
        print("\n=== Execute Command ===")
        client.execute_command("render")

    except KeyboardInterrupt:
        print("\nInterrupted by user")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        # Disconnect
        client.disconnect()


if __name__ == "__main__":
    main()
