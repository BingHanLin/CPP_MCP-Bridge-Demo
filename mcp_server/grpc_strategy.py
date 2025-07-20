#!/usr/bin/env python3
"""
gRPC Communication Strategy

This module implements gRPC-based communication with C++ software
using Protocol Buffers over HTTP/2 for high-performance communication.

Author: AI Assistant
Version: 1.0.0
"""

import json
import logging
from typing import Any, Dict, Optional

import grpc
from .communication_strategy import CommunicationStrategy
from . import mcp_service_pb2
from . import mcp_service_pb2_grpc

logger = logging.getLogger(__name__)


class GrpcStrategy(CommunicationStrategy):
    """gRPC-based communication strategy."""

    def __init__(self, address: str = "localhost:50051"):
        self.address = address
        self.channel = None
        self.stub = None
        self.connected = False
        logger.info(f"Initialized GrpcStrategy for {address}")

    async def connect(self) -> bool:
        """Connect to the C++ software via gRPC."""
        try:
            logger.info(f"Attempting gRPC connection to {self.address}...")

            # Create gRPC channel and stub
            self.channel = grpc.aio.insecure_channel(self.address)
            self.stub = mcp_service_pb2_grpc.MCPServiceStub(self.channel)

            # Test connection by calling a simple method
            request = mcp_service_pb2.GetSoftwareInfoRequest()
            try:
                response = await self.stub.GetSoftwareInfo(request, timeout=5.0)
                self.connected = True
                logger.info("gRPC connection established successfully")
                return True
            except grpc.RpcError as e:
                logger.error(f"gRPC connection test failed: {e}")
                self.connected = False
                return False

        except Exception as e:
            logger.error(f"gRPC connection failed: {e}")
            self.connected = False
            return False

    async def disconnect(self) -> bool:
        """Disconnect from gRPC service."""
        try:
            if self.channel:
                await self.channel.close()
                self.channel = None
            self.stub = None
            self.connected = False
            logger.info("gRPC disconnected successfully")
            return True
        except Exception as e:
            logger.error(f"gRPC disconnection error: {e}")
            return False

    async def send_command(self, command: str, params: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """Send command via gRPC and return response."""
        if not self.connected:
            return {
                "success": False,
                "error": "Not connected to gRPC service"
            }

        try:
            logger.debug(f"Sending gRPC command: {command}")

            if command == "get_software_info":
                return await self._get_software_info()
            elif command == "get_software_status":
                return await self._get_software_status()
            elif command == "create_object":
                return await self._create_object(params or {})
            elif command == "delete_object":
                return await self._delete_object(params or {})
            elif command == "list_objects":
                return await self._list_objects()
            elif command == "get_object_info":
                return await self._get_object_info(params or {})
            elif command == "execute_software_command":
                return await self._execute_software_command(params or {})
            elif command == "save_project":
                return await self._save_project(params or {})
            elif command == "load_project":
                return await self._load_project(params or {})
            else:
                return {
                    "success": False,
                    "error": f"Unknown command: {command}"
                }

        except Exception as e:
            logger.error(f"gRPC command failed: {e}")
            return {
                "success": False,
                "error": f"gRPC communication error: {str(e)}"
            }

    async def _get_software_info(self) -> Dict[str, Any]:
        """Get software information."""
        request = mcp_service_pb2.GetSoftwareInfoRequest()
        response = await self.stub.GetSoftwareInfo(request)

        return {
            "success": True,
            "software_name": response.info.software_name,
            "version": response.info.version,
            "status": response.info.status,
            "current_project": response.info.current_project,
            "total_objects": response.info.total_objects,
            "available_commands": list(response.info.available_commands)
        }

    async def _get_software_status(self) -> Dict[str, Any]:
        """Get software status."""
        request = mcp_service_pb2.GetSoftwareStatusRequest()
        response = await self.stub.GetSoftwareStatus(request)

        return {
            "success": True,
            "running": response.status.running,
            "current_project": response.status.current_project,
            "object_count": response.status.object_count,
            "memory_usage": response.status.memory_usage,
            "uptime": response.status.uptime
        }

    async def _create_object(self, params: Dict[str, Any]) -> Dict[str, Any]:
        """Create an object."""
        request = mcp_service_pb2.CreateObjectRequest()
        request.name = params.get("name", "new_object")
        request.type = params.get("type", "cube")

        # Add properties
        for key, value in params.items():
            if key not in ["name", "type"]:
                prop = request.properties.add()
                prop.key = key
                prop.value = str(value)

        response = await self.stub.CreateObject(request)

        result = {
            "success": response.success,
            "object_id": response.object_id
        }

        if response.error:
            result["error"] = response.error

        if response.object:
            result["object"] = {
                "name": response.object.name,
                "type": response.object.type,
                "properties": {prop.key: prop.value for prop in response.object.properties}
            }

        return result

    async def _delete_object(self, params: Dict[str, Any]) -> Dict[str, Any]:
        """Delete an object."""
        request = mcp_service_pb2.DeleteObjectRequest()
        request.object_id = params.get("id", "")

        response = await self.stub.DeleteObject(request)

        result = {
            "success": response.success,
            "message": response.message
        }

        if response.error:
            result["error"] = response.error

        return result

    async def _list_objects(self) -> Dict[str, Any]:
        """List all objects."""
        request = mcp_service_pb2.ListObjectsRequest()
        response = await self.stub.ListObjects(request)

        objects = []
        for obj in response.objects:
            objects.append({
                "id": obj.id,
                "name": obj.name,
                "type": obj.type
            })

        return {
            "success": True,
            "total_count": response.total_count,
            "objects": objects
        }

    async def _get_object_info(self, params: Dict[str, Any]) -> Dict[str, Any]:
        """Get object information."""
        request = mcp_service_pb2.GetObjectInfoRequest()
        request.object_id = params.get("id", "")

        response = await self.stub.GetObjectInfo(request)

        result = {
            "success": response.success
        }

        if response.error:
            result["error"] = response.error

        if response.object:
            result["object"] = {
                "name": response.object.name,
                "type": response.object.type,
                "properties": {prop.key: prop.value for prop in response.object.properties}
            }

        return result

    async def _execute_software_command(self, params: Dict[str, Any]) -> Dict[str, Any]:
        """Execute a software command."""
        request = mcp_service_pb2.ExecuteSoftwareCommandRequest()
        request.command = params.get("command", "")

        # Add parameters
        for key, value in params.items():
            if key != "command":
                prop = request.params.add()
                prop.key = key
                prop.value = str(value)

        response = await self.stub.ExecuteSoftwareCommand(request)

        result = {
            "success": response.success,
            "message": response.message
        }

        if response.error:
            result["error"] = response.error

        if response.output_file:
            result["output_file"] = response.output_file

        return result

    async def _save_project(self, params: Dict[str, Any]) -> Dict[str, Any]:
        """Save project."""
        request = mcp_service_pb2.SaveProjectRequest()
        request.filename = params.get("filename", "")

        response = await self.stub.SaveProject(request)

        result = {
            "success": response.success,
            "message": response.message,
            "filename": response.filename
        }

        if response.error:
            result["error"] = response.error

        return result

    async def _load_project(self, params: Dict[str, Any]) -> Dict[str, Any]:
        """Load project."""
        request = mcp_service_pb2.LoadProjectRequest()
        request.filename = params.get("filename", "")

        response = await self.stub.LoadProject(request)

        result = {
            "success": response.success,
            "message": response.message,
            "filename": response.filename,
            "objects_loaded": response.objects_loaded
        }

        if response.error:
            result["error"] = response.error

        return result

    def is_connected(self) -> bool:
        """Check if gRPC is connected."""
        return self.connected

    def get_strategy_info(self) -> Dict[str, Any]:
        """Get gRPC strategy information."""
        return {
            "type": "grpc",
            "address": self.address,
            "connected": self.is_connected()
        }
