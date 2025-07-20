#!/usr/bin/env python3
"""
Unified MCP Server - C++ Software Control Bridge

This server provides a unified interface to communicate with C++ software
via either socket or gRPC communication strategies. It implements the
Model Context Protocol (MCP) for integration with AI assistants.

Features:
- Dynamic communication mode switching (socket/gRPC)
- Software state management
- Object creation/manipulation
- Project management
- Command execution
- Error handling and status monitoring

Communication Modes:
- Socket: Direct TCP socket communication
- gRPC: Protocol Buffers over HTTP/2

Author: AI Assistant
Version: 1.0.0
"""

import asyncio
import json
import logging
import sys
from typing import Any, Dict, List, Optional

from fastmcp import FastMCP

from .communication_strategy import CommunicationStrategy
from .socket_strategy import SocketStrategy
from .grpc_strategy import GrpcStrategy

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# ============================================================================
# Global Variables for Current Strategy
# ============================================================================

# Global strategy instances and current state
current_strategy: Optional[CommunicationStrategy] = None
current_mode: str = "socket"
socket_strategy: Optional[SocketStrategy] = None
grpc_strategy: Optional[GrpcStrategy] = None

# Initialize FastMCP server
mcp = FastMCP("Unified C++ MCP Bridge Server")

# ============================================================================
# Helper Functions
# ============================================================================


async def set_communication_mode_internal(mode: str) -> Dict[str, Any]:
    """Internal function to switch communication mode."""
    global current_strategy, current_mode, socket_strategy, grpc_strategy

    if mode not in ["socket", "grpc"]:
        return {
            "success": False,
            "error": f"Invalid mode '{mode}'. Must be 'socket' or 'grpc'"
        }

    if mode == current_mode:
        return {
            "success": True,
            "message": f"Already using {mode} mode",
            "mode": mode
        }

    try:
        # Disconnect current strategy if it exists
        if current_strategy:
            await current_strategy.disconnect()

        # Switch to new strategy
        if mode == "grpc":
            current_strategy = grpc_strategy
        else:
            current_strategy = socket_strategy

        current_mode = mode
        # Try to connect with new strategy
        if current_strategy:
            connected = await current_strategy.connect()
        else:
            connected = False

        return {
            "success": True,
            "message": f"Switched to {mode} mode",
            "mode": mode,
            "connected": connected
        }

    except Exception as e:
        logger.error(f"Failed to switch communication mode: {e}")
        return {
            "success": False,
            "error": f"Failed to switch to {mode} mode: {str(e)}"
        }


def get_software_connection_internal() -> Dict[str, Any]:
    """Internal function to get current software connection status."""
    global current_strategy, current_mode

    if not current_strategy:
        return {
            "current_mode": current_mode,
            "strategy_info": {"type": "unknown", "connected": False},
            "available_modes": ["socket", "grpc"]
        }

    strategy_info = current_strategy.get_strategy_info()
    return {
        "current_mode": current_mode,
        "strategy_info": strategy_info,
        "available_modes": ["socket", "grpc"]
    }

# ============================================================================
# MCP Tools Implementation
# ============================================================================


@mcp.tool()
async def set_communication_mode(mode: str) -> str:
    """
    Switch the communication mode between socket and gRPC.

    Args:
        mode: The target communication mode ('socket' or 'grpc')
    """
    if not current_strategy or not socket_strategy or not grpc_strategy:
        return "Error: Server not initialized"

    result = await set_communication_mode_internal(mode)
    return json.dumps(result, indent=2)


@mcp.tool()
async def get_software_connection() -> str:
    """
    Get the current software connection status and available communication modes.
    """
    result = get_software_connection_internal()
    return json.dumps(result, indent=2)


@mcp.tool()
async def get_software_status() -> str:
    """
    Get the current status of the C++ software.
    """
    if not current_strategy:
        return "Error: Server not initialized"

    result = await current_strategy.execute_software_command("get_status")
    return json.dumps(result, indent=2)


@mcp.tool()
async def get_software_info() -> str:
    """
    Get detailed information about the C++ software.
    """
    if not current_strategy:
        return "Error: Server not initialized"

    result = await current_strategy.execute_software_command("get_info")
    return json.dumps(result, indent=2)


@mcp.tool()
async def execute_software_command(command: str, params: str = "{}") -> str:
    """
    Execute a software-specific command.

    Args:
        command: The command to execute (render, clear_scene, reset_camera, etc.)
        params: JSON string containing additional command parameters
    """
    if not current_strategy:
        return "Error: Server not initialized"

    try:
        # Parse params from JSON string
        import json as json_module
        kwargs = json_module.loads(params) if params != "{}" else {}
    except json_module.JSONDecodeError:
        return "Error: Invalid JSON in params parameter"

    result = await current_strategy.execute_software_command(command, **kwargs)
    return json.dumps(result, indent=2)


@mcp.tool()
async def create_object(name: str, object_type: str, properties: str = "{}") -> str:
    """
    Create a new object in the software.

    Args:
        name: Name of the object to create
        object_type: Type of object (cube, sphere, camera, etc.)
        properties: JSON string containing additional object properties
    """
    if not current_strategy:
        return "Error: Server not initialized"

    try:
        # Parse properties from JSON string
        import json as json_module
        kwargs = json_module.loads(properties) if properties != "{}" else {}
    except json_module.JSONDecodeError:
        return "Error: Invalid JSON in properties parameter"

    params = {"name": name, "object_type": object_type, **kwargs}
    result = await current_strategy.execute_software_command("create_object", **params)
    return json.dumps(result, indent=2)


@mcp.tool()
async def delete_object(object_id: str) -> str:
    """
    Delete an object from the software.

    Args:
        object_id: ID of the object to delete
    """
    if not current_strategy:
        return "Error: Server not initialized"

    result = await current_strategy.execute_software_command("delete_object", object_id=object_id)
    return json.dumps(result, indent=2)


@mcp.tool()
async def get_object_info(object_id: str) -> str:
    """
    Get detailed information about a specific object.

    Args:
        object_id: ID of the object to get info about
    """
    if not current_strategy:
        return "Error: Server not initialized"

    result = await current_strategy.execute_software_command("get_object_info", object_id=object_id)
    return json.dumps(result, indent=2)


@mcp.tool()
async def list_objects() -> str:
    """
    List all objects in the software.
    """
    if not current_strategy:
        return "Error: Server not initialized"

    result = await current_strategy.execute_software_command("list_objects")
    return json.dumps(result, indent=2)


@mcp.tool()
async def save_project(filename: Optional[str] = None) -> str:
    """
    Save the current project.

    Args:
        filename: Optional filename to save to
    """
    if not current_strategy:
        return "Error: Server not initialized"

    params = {"filename": filename} if filename else {}
    result = await current_strategy.execute_software_command("save_project", **params)
    return json.dumps(result, indent=2)


@mcp.tool()
async def load_project(filename: str) -> str:
    """
    Load a project from file.

    Args:
        filename: Name of the project file to load
    """
    if not current_strategy:
        return "Error: Server not initialized"

    result = await current_strategy.execute_software_command("load_project", filename=filename)
    return json.dumps(result, indent=2)

# ============================================================================
# Main Function and CLI
# ============================================================================


def main():
    """Main entry point for the unified MCP server."""
    import argparse

    parser = argparse.ArgumentParser(
        description="Unified MCP Server - C++ Software Bridge")
    parser.add_argument("--mode", choices=["socket", "grpc"], default="socket",
                        help="Communication mode (default: socket)")
    parser.add_argument("--socket-host", default="localhost",
                        help="Socket host (default: localhost)")
    parser.add_argument("--socket-port", type=int, default=9876,
                        help="Socket port (default: 9876)")
    parser.add_argument("--grpc-address", default="localhost:50051",
                        help="gRPC address (default: localhost:50051)")
    parser.add_argument("--log-level", choices=["DEBUG", "INFO", "WARNING", "ERROR"],
                        default="INFO", help="Log level (default: INFO)")

    args = parser.parse_args()

    # Set log level
    logging.getLogger().setLevel(getattr(logging, args.log_level))

    # Initialize global variables in main() function
    global current_strategy, current_mode, socket_strategy, grpc_strategy

    # Create strategy instances in main()
    socket_strategy = SocketStrategy(args.socket_host, args.socket_port)
    grpc_strategy = GrpcStrategy(args.grpc_address)

    # Set initial strategy
    if args.mode == "grpc":
        current_strategy = grpc_strategy
    else:
        current_strategy = socket_strategy

    current_mode = args.mode

    logger.info("=" * 60)
    logger.info("  Unified MCP Server - C++ Software Bridge")
    logger.info("=" * 60)
    logger.info(f"Mode: {args.mode}")
    if args.mode == "socket":
        logger.info(f"Socket: {args.socket_host}:{args.socket_port}")
    else:
        logger.info(f"gRPC: {args.grpc_address}")
    logger.info("=" * 60)

    # Run the MCP server
    mcp.run()


if __name__ == "__main__":
    main()
