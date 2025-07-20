# C++ MCP Bridge Demo

This project demonstrates how to create a bridge between C++ applications and AI assistants using a Python MCP server with support for both Socket and gRPC communication.

## Features

-   **Python MCP Server**: Implementing Model Context Protocol for AI assistant integration
-   **C++ Application**: Core business logic with command processing
-   **Dual Communication**: Both Socket and gRPC protocols supported

## Quick Start

### Prerequisites

-   **C++ Compiler**: Visual Studio 2019+ (tested on Windows)
-   **CMake**: 3.10 or higher
-   **Python**: 3.10 or higher with UV package manager

### Build & Setup

```bash
# 1. Clone and navigate to the project
git clone <repository-url>
cd cpp-mcp-bridge-demo

# 2. Build C++ application
cmake --preset MSVC_x64-debug
cmake --build --preset MSVC_x64-debug

# 3. Install Python dependencies
uv venv
uv pip install -r requirements.txt
```

### Running the Server

#### Socket Mode (Default)

```bash
# Start C++ backend (socket server, default port 9876)
.\bin\cpp_app.exe socket 9876

# Start MCP server (in another terminal)
uv run mcp-server-demo --mode socket --socket-host localhost --socket-port 9876
```

#### gRPC Mode

```bash
# Start C++ backend (gRPC server, default 0.0.0.0:50051)
.\bin\cpp_app.exe grpc localhost:50051

# Start MCP server (in another terminal)
uv run mcp-server-demo --mode grpc --grpc-address localhost:50051
```

## 📋 Available Commands

### Object Management

-   `create_object(name, type, properties)`: Create objects (cube, sphere, camera)
-   `delete_object(object_id)`: Delete objects by ID
-   `list_objects()`: List all objects in the scene
-   `get_object_info(object_id)`: Get detailed object information

### Software Operations

-   `get_software_info()`: Get software information and version
-   `get_software_status()`: Get current software status
-   `execute_software_command(command, params)`: Execute commands (render, clear_scene, reset_camera)

### Project Management

-   `save_project(filename)`: Save current project to file
-   `load_project(filename)`: Load project from file

## 🏗️ Architecture

### Communication Flow

#### Socket Mode

```
AI Assistant ←→ Python MCP Server ←→ C++ Application (Socket)
```

#### gRPC Mode

```
AI Assistant ←→ Python MCP Server ←→ C++ Application (gRPC)
```

### Command Format

**Request**:

```json
{
    "type": "command_name",
    "params": {
        "param1": "value1",
        "param2": "value2"
    }
}
```

**Response**:

```json
{
  "status": "success|error",
  "result": {...},
  "message": "error message if applicable"
}
```

## 🔧 Development

### Project Structure

-   **`cpp_app/`**: Core C++ application with business logic
-   **`mcp_server/`**: Python MCP server implementation
-   **`proto/`**: Protocol buffer definitions for gRPC

### Adding New Commands

1. **C++ Side**: Add method to `commandHandler` class
2. **Python Side**: Add corresponding MCP tool function in `server.py`

## 📄 License

MIT License - Feel free to use and modify for your own projects.

## 🔗 References

-   [Model Context Protocol (MCP)](https://modelcontextprotocol.io/)
-   [FastMCP](https://github.com/jlowin/fastmcp) - Python MCP framework
