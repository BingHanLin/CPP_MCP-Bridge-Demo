# C++ MCP Bridge Demo

This project demonstrates how to create a bridge between C++ applications and AI assistants using a Python MCP server. It now includes **gRPC support** for cross-language communication.

## 🚀 Features

-   **JSON-based MCP Server**: Python server implementing Model Context Protocol
-   **C++ Application**: Core business logic with command processing
-   **gRPC Communication**: High-performance protocol buffer communication
-   **Multi-protocol Support**: Both socket-based and gRPC communication
-   **Cross-platform**: Windows, Linux, and macOS support

## 🛠️ Build

### Prerequisites

-   **C++ Compiler**:
    -   Windows: Visual Studio 2019 or later (MSVC)
    -   Linux: GCC 7+ or Clang 6+
    -   macOS: Xcode 10+ or Clang 6+
-   **CMake**: 3.10 or higher
-   **Python**: 3.10 or higher
-   **UV**: Package manager (recommended) or pip

### Setup Steps

1. **Clone the repository**:

```bash
git clone <repository-url>
cd cpp-mcp-bridge-demo
```

2. **Build C++ application using CMake Presets**:

```bash
cmake --preset MSVC_x64-debug
cmake --build --preset MSVC_x64-debug
```

3. **Install Python dependencies**:

```bash
uv venv
uv pip install -r requirements.txt
```

## 🎯 Usage

### Quick Start

#### Unified Server (Recommended)

The new unified server supports both Socket and gRPC communication through a single executable:

```bash
# Build the project
cmake --preset MSVC_x64-debug
cmake --build --preset MSVC_x64-debug

# Run with Socket server (default)
.\build\bin\cpp_app_unified.exe
.\build\bin\cpp_app_unified.exe socket 9876

# Run with gRPC server (if gRPC is available)
.\build\bin\cpp_app_unified.exe grpc
.\build\bin\cpp_app_unified.exe grpc localhost:50051

# Get help
.\build\bin\cpp_app_unified.exe help
```

#### Legacy Options (Still Available)

1. **Start the C++ application**:

    ```bash
    .\build\release\MSVC_x64-release\bin\cpp_app.exe
    ```

2. **Start the MCP server**:

    ```bash
    uv run  mcp-server-demo
    ```

3. **Use with your preferred MCP client**

#### Option 2: gRPC Communication (New!)

1. **Start the gRPC server**:

    ```bash
    .\build\release\MSVC_x64-release\bin\cpp_app_grpc_server.exe
    ```

2. **Connect using any gRPC client** (Python example provided in `examples/`)

For detailed gRPC setup instructions, see [GRPC_SETUP.md](GRPC_SETUP.md).

### Available Commands

#### Basic Information

-   `get_software_info()`: Get software information and version
-   `get_software_status()`: Get current software status and statistics

#### Object Management

-   `create_object(name, type, **kwargs)`: Create new objects (cube, sphere, camera)
-   `delete_object(object_id)`: Delete objects by ID
-   `list_objects()`: List all objects in the scene
-   `get_object_info(object_id)`: Get detailed object information

#### Project Management

-   `save_project(filename)`: Save current project to file
-   `load_project(filename)`: Load project from file

#### Software Operations

-   `execute_software_command(command, **kwargs)`: Execute software-specific commands
    -   `render`: Render the current scene
    -   `clear_scene`: Clear all objects
    -   `reset_camera`: Reset camera to default position

## 🏗️ Architecture

### Communication Flow

#### Original Architecture

```
MCP Client ←→ Python MCP Server ←→ C++ Application (Socket)
```

#### New gRPC Architecture

```
gRPC Client ←→ C++ gRPC Server ←→ C++ Application Core
     ↓
Any Language (Python, Node.js, Go, etc.)
```

### Key Components

-   **`cpp_app/`**: Core C++ application
    -   `softwareCore`: Business logic and data management
    -   `commandHandler`: Command parsing and JSON conversion
    -   `commandServer`: Socket-based communication server
    -   `grpcMCPService`: gRPC service implementation (new)
    -   `grpcServer`: gRPC server main program (new)
-   **`mcp_server/`**: Python MCP server implementation
-   **`proto/`**: Protocol buffer definitions (new)
-   **`examples/`**: Client examples and demos (new)

### Communication Protocol

**Command Format**:

```json
{
    "type": "command_name",
    "params": {
        "param1": "value1",
        "param2": "value2"
    }
}
```

**Response Format**:

```json
{
  "status": "success|error",
  "result": {...},
  "message": "error message if applicable"
}
```

## 🔧 Development

### Adding New Commands

1. **C++ Side**:

    - Add method to `commandHandler` class in `commandHandler.hpp`
    - Implement the method in `commandHandler.cpp`
    - Register handler in `main.cpp`

2. **Python Side**:
    - Add corresponding MCP tool function in `server.py`
    - Update documentation and prompts

### Example: Adding a "render" command

**C++ (commandHandler.hpp)**:

```cpp
nlohmann::json renderScene(const nlohmann::json& params);
```

**C++ (commandHandler.cpp)**:

```cpp
nlohmann::json commandHandler::renderScene(const nlohmann::json& params) {
    // Implementation here
    return {{"success", true}, {"output", "render.png"}};
}
```

**Python (server.py)**:

```python
@mcp.tool()
def render_scene(ctx: Context, output_file: str = "render.png") -> str:
    """Render the current scene"""
    try:
        software = get_software_connection()
        result = software.send_command("render_scene", {"output_file": output_file})
        return json.dumps(result, indent=2)
    except Exception as e:
        return f"Error rendering scene: {str(e)}"
```

## 📄 License

MIT License - Feel free to use and modify for your own projects.

## 🔗 References

-   [Model Context Protocol (MCP)](https://modelcontextprotocol.io/)
-   [BlenderMCP](https://github.com/rikhuijzer/blender-mcp) - Inspiration for this architecture
-   [FastMCP](https://github.com/jlowin/fastmcp) - Python MCP framework
-   [nlohmann/json](https://github.com/nlohmann/json) - JSON library for C++
