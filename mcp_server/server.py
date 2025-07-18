# my_software_mcp_server.py
from mcp.server.fastmcp import FastMCP, Context
import socket
import json
import asyncio
import logging
from dataclasses import dataclass
from contextlib import asynccontextmanager
from typing import AsyncIterator, Dict, Any

# Configure logging
logging.basicConfig(level=logging.INFO, 
                    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger("MySoftwareMCPServer")

@dataclass
class SoftwareConnection:
    host: str
    port: int
    sock: socket.socket = None
    
    def connect(self) -> bool:
        """Connect to the C++ software socket server"""
        if self.sock:
            return True
            
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.host, self.port))
            logger.info(f"Connected to software at {self.host}:{self.port}")
            return True
        except Exception as e:
            logger.error(f"Failed to connect to software: {str(e)}")
            self.sock = None
            return False
    
    def disconnect(self):
        """Disconnect from the software"""
        if self.sock:
            try:
                self.sock.close()
            except Exception as e:
                logger.error(f"Error disconnecting from software: {str(e)}")
            finally:
                self.sock = None

    def send_command(self, command_type: str, params: Dict[str, Any] = None) -> Dict[str, Any]:
        """Send a command to the software and return the response"""
        if not self.sock and not self.connect():
            raise ConnectionError("Not connected to software")
        
        command = {
            "type": command_type,
            "params": params or {}
        }
        
        try:
            # Log the command being sent
            logger.info(f"Sending command: {command_type} with params: {params}")
            
            # Send the command
            command_str = json.dumps(command)
            self.sock.sendall(command_str.encode('utf-8'))
            logger.info(f"Command sent, waiting for response...")
            
            # Receive the response
            response_data = self._receive_full_response()
            logger.info(f"Received {len(response_data)} bytes of data")
            
            response = json.loads(response_data.decode('utf-8'))
            logger.info(f"Response parsed, status: {response.get('status', 'unknown')}")
            
            if response.get("status") == "error":
                logger.error(f"Software error: {response.get('message')}")
                raise Exception(response.get("message", "Unknown error from software"))
            
            return response.get("result", {})
        except socket.timeout:
            logger.error("Socket timeout while waiting for response from software")
            self.sock = None
            raise Exception("Timeout waiting for software response")
        except (ConnectionError, BrokenPipeError, ConnectionResetError) as e:
            logger.error(f"Socket connection error: {str(e)}")
            self.sock = None
            raise Exception(f"Connection to software lost: {str(e)}")
        except json.JSONDecodeError as e:
            logger.error(f"Invalid JSON response from software: {str(e)}")
            raise Exception(f"Invalid response from software: {str(e)}")
        except Exception as e:
            logger.error(f"Error communicating with software: {str(e)}")
            self.sock = None
            raise Exception(f"Communication error with software: {str(e)}")

    def _receive_full_response(self, buffer_size=8192):
        """Receive the complete response, potentially in multiple chunks"""
        chunks = []
        self.sock.settimeout(15.0)  # Set timeout
        
        while True:
            try:
                chunk = self.sock.recv(buffer_size)
                if not chunk:
                    break
                
                chunks.append(chunk)
                
                # Check if we have a complete JSON object
                try:
                    data = b''.join(chunks)
                    json.loads(data.decode('utf-8'))
                    return data
                except json.JSONDecodeError:
                    # Incomplete JSON, continue receiving
                    continue
            except socket.timeout:
                logger.warning("Socket timeout during receive")
                break
        
        # Try to use what we have
        if chunks:
            data = b''.join(chunks)
            try:
                json.loads(data.decode('utf-8'))
                return data
            except json.JSONDecodeError:
                raise Exception("Incomplete JSON response received")
        else:
            raise Exception("No data received")

@asynccontextmanager
async def server_lifespan(server: FastMCP) -> AsyncIterator[Dict[str, Any]]:
    """Manage server startup and shutdown lifecycle"""
    try:
        logger.info("MySoftwareMCP server starting up")
        
        # Try to connect to software on startup
        try:
            software = get_software_connection()
            logger.info("Successfully connected to software on startup")
        except Exception as e:
            logger.warning(f"Could not connect to software on startup: {str(e)}")
            logger.warning("Make sure the C++ software is running before using tools")
        
        yield {}
    finally:
        # Clean up connection on shutdown
        global _software_connection
        if _software_connection:
            logger.info("Disconnecting from software on shutdown")
            _software_connection.disconnect()
            _software_connection = None
        logger.info("MySoftwareMCP server shut down")

# Create the MCP server with lifespan support
mcp = FastMCP(
    "MySoftwareMCP",
    description="My Software integration through the Model Context Protocol",
    lifespan=server_lifespan
)

# Global connection
_software_connection = None

def get_software_connection():
    """Get or create a persistent software connection"""
    global _software_connection
    
    # If we have an existing connection, check if it's still valid
    if _software_connection is not None:
        try:
            # Test connection with a simple command
            result = _software_connection.send_command("get_software_status")
            return _software_connection
        except Exception as e:
            logger.warning(f"Existing connection is no longer valid: {str(e)}")
            try:
                _software_connection.disconnect()
            except:
                pass
            _software_connection = None
    
    # Create a new connection if needed
    if _software_connection is None:
        _software_connection = SoftwareConnection(host="localhost", port=9876)
        if not _software_connection.connect():
            logger.error("Failed to connect to software")
            _software_connection = None
            raise Exception("Could not connect to software. Make sure the C++ application is running.")
        logger.info("Created new persistent connection to software")
    
    return _software_connection

# MCP Tools

@mcp.tool()
def get_software_info(ctx: Context) -> str:
    """Get detailed information about the software"""
    try:
        software = get_software_connection()
        result = software.send_command("get_software_info")
        return json.dumps(result, indent=2)
    except Exception as e:
        logger.error(f"Error getting software info: {str(e)}")
        return f"Error getting software info: {str(e)}"

@mcp.tool()
def get_software_status(ctx: Context) -> str:
    """Get current software status"""
    try:
        software = get_software_connection()
        result = software.send_command("get_software_status")
        return json.dumps(result, indent=2)
    except Exception as e:
        logger.error(f"Error getting software status: {str(e)}")
        return f"Error getting software status: {str(e)}"

@mcp.tool()
def create_object(ctx: Context, name: str, object_type: str, **kwargs) -> str:
    """
    Create a new object in the software
    
    Parameters:
    - name: Name of the object to create
    - object_type: Type of object (cube, sphere, camera, etc.)
    - **kwargs: Additional object properties
    """
    try:
        software = get_software_connection()
        params = {
            "name": name,
            "type": object_type,
            **kwargs
        }
        result = software.send_command("create_object", params)
        return json.dumps(result, indent=2)
    except Exception as e:
        logger.error(f"Error creating object: {str(e)}")
        return f"Error creating object: {str(e)}"

@mcp.tool()
def delete_object(ctx: Context, object_id: str) -> str:
    """
    Delete an object from the software
    
    Parameters:
    - object_id: ID of the object to delete
    """
    try:
        software = get_software_connection()
        result = software.send_command("delete_object", {"id": object_id})
        return json.dumps(result, indent=2)
    except Exception as e:
        logger.error(f"Error deleting object: {str(e)}")
        return f"Error deleting object: {str(e)}"

@mcp.tool()
def list_objects(ctx: Context) -> str:
    """List all objects in the software"""
    try:
        software = get_software_connection()
        result = software.send_command("list_objects")
        return json.dumps(result, indent=2)
    except Exception as e:
        logger.error(f"Error listing objects: {str(e)}")
        return f"Error listing objects: {str(e)}"

@mcp.tool()
def get_object_info(ctx: Context, object_id: str) -> str:
    """
    Get detailed information about a specific object
    
    Parameters:
    - object_id: ID of the object to get info about
    """
    try:
        software = get_software_connection()
        result = software.send_command("get_object_info", {"id": object_id})
        return json.dumps(result, indent=2)
    except Exception as e:
        logger.error(f"Error getting object info: {str(e)}")
        return f"Error getting object info: {str(e)}"

@mcp.tool()
def execute_software_command(ctx: Context, command: str, **kwargs) -> str:
    """
    Execute a software-specific command
    
    Parameters:
    - command: The command to execute (render, clear_scene, reset_camera, etc.)
    - **kwargs: Additional command parameters
    """
    try:
        software = get_software_connection()
        params = {
            "command": command,
            **kwargs
        }
        result = software.send_command("execute_software_command", params)
        return json.dumps(result, indent=2)
    except Exception as e:
        logger.error(f"Error executing command: {str(e)}")
        return f"Error executing command: {str(e)}"

@mcp.tool()
def save_project(ctx: Context, filename: str = None) -> str:
    """
    Save the current project
    
    Parameters:
    - filename: Optional filename to save to
    """
    try:
        software = get_software_connection()
        params = {}
        if filename:
            params["filename"] = filename
        result = software.send_command("save_project", params)
        return json.dumps(result, indent=2)
    except Exception as e:
        logger.error(f"Error saving project: {str(e)}")
        return f"Error saving project: {str(e)}"

@mcp.tool()
def load_project(ctx: Context, filename: str) -> str:
    """
    Load a project from file
    
    Parameters:
    - filename: Name of the project file to load
    """
    try:
        software = get_software_connection()
        result = software.send_command("load_project", {"filename": filename})
        return json.dumps(result, indent=2)
    except Exception as e:
        logger.error(f"Error loading project: {str(e)}")
        return f"Error loading project: {str(e)}"

@mcp.prompt()
def software_usage_guide() -> str:
    """Provides guidance on using the software through MCP"""
    return """When working with My Software through MCP, follow these guidelines:

1. **Software Status**: Always start by checking the software status using get_software_status()
2. **Object Management**: 
   - Create objects using create_object() with name and type
   - List all objects with list_objects()
   - Get detailed info with get_object_info()
   - Delete objects with delete_object()
3. **Available Object Types**:
   - cube: Creates a cube object (properties: size, color)
   - sphere: Creates a sphere object (properties: radius, color) 
   - camera: Creates a camera object (properties: position, rotation)
4. **Software Commands**:
   - render: Render the current scene
   - clear_scene: Clear all objects from the scene
   - reset_camera: Reset camera to default position
5. **Project Management**:
   - Save your work with save_project()
   - Load existing projects with load_project()

Always check the software status first to ensure proper connection."""

# Main execution
def main():
    """Run the MCP server"""
    mcp.run()

if __name__ == "__main__":
    main()
