# C++ MCP Bridge Demo

A comprehensive example of integrating C++ software with Claude AI through the Model Context Protocol (MCP). This project demonstrates how to create a bridge between C++ applications and AI assistants using a Python MCP server.

## 🚀 Project Overview

This is a complete, modular MCP server project that showcases how to integrate C++ software with Claude AI. It provides a clean architecture for two-way communication between C++ applications and AI assistants.

## 📁 Project Structure

```
cpp-mcp-bridge-demo/
├── README.md                    # This file
├── requirements.txt             # Python dependencies
├── pyproject.toml              # Python project configuration
├── uv.lock                     # UV lock file
├── Allmake                     # Build script
├── .vscode/                    # VS Code configuration
│   └── mcp.json               # MCP configuration
├── cpp_app/                    # C++ application
│   ├── main.cpp               # Main application entry point
│   ├── commandHandler.hpp     # Command handler header
│   ├── commandHandler.cpp     # Command handler implementation
│   ├── commandServer.hpp      # Command server header
│   ├── commandServer.cpp      # Command server implementation
│   ├── CMakeLists.txt         # CMake build configuration
│   └── nlohmann/              # JSON library
│       └── json.hpp           # JSON header
├── mcp_server/                 # Python MCP server
│   ├── __init__.py
│   └── server.py              # MCP server implementation
├── build/                      # Build output directory
│   └── bin/
│       └── cpp_app.exe        # Compiled C++ application
└── current_project.json       # Sample project file
```

## 🛠️ Build

### Prerequisites

-   **C++ Compiler**: GCC/Clang/MSVC with C++17 support
-   **CMake**: 3.10 or higher
-   **Python**: 3.10 or higher
-   **UV**: Package manager (recommended) or pip

### Setup Steps

1. **Clone the repository**:

```bash
git clone <repository-url>
cd cpp-mcp-bridge-demo
```

2. **Build C++ application**:

```bash
# Windows
mkdir build
cd build
cmake ..
cmake --build . --config Release

# Linux/macOS
mkdir build && cd build
cmake ..
make
```

3. **Install Python dependencies**:

```bash
# Using UV (recommended)
uv venv
uv pip install -r requirements.txt

# Using pip
pip install -r requirements.txt
```

4. **Configure Claude Desktop**:
   Add the MCP server configuration to your Claude desktop config.

## 🎯 Usage

### Quick Start

1. **Start the C++ application**:

```bash
cd build/bin
./cpp_app.exe  # Windows
./cpp_app      # Linux/macOS
```

2. **Start the MCP server**:

```bash
python -m mcp_server.server
```

3. **Use with Claude**:
   The MCP server will automatically connect to the C++ application and be available in Claude.

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

### Example Usage in Claude

```
User: "Check the software status"
Claude: Calls get_software_status() and displays results

User: "Create a red cube named 'my_cube'"
Claude: Calls create_object(name="my_cube", type="cube", color="red")

User: "List all objects in the scene"
Claude: Calls list_objects() and formats the display

User: "Save the current project as 'my_scene.json'"
Claude: Calls save_project(filename="my_scene.json")

User: "Render the scene"
Claude: Calls execute_software_command(command="render")
```

## 🏗️ Architecture

### Communication Flow

```
MCP Client ←→ Python MCP Server ←→ C++ Application
```

### Key Components

1. **C++ Application** (`cpp_app/`)

    - `main.cpp`: Entry point and command handler registration
    - `commandHandler.hpp/.cpp`: Software-specific command implementations
    - `commandServer.hpp/.cpp`: Generic socket server for MCP communication
    - `CMakeLists.txt`: Build configuration

2. **Python MCP Server** (`mcp_server/`)
    - `server.py`: MCP server implementation with connection management
    - `__init__.py`: Package initialization

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
