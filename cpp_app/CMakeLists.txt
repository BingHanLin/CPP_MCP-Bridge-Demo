
project(cpp_app)

# Find required packages
find_package(Threads REQUIRED)

# Core source files (common to all executables)
set(SOURCES
    ${PROJECT_SOURCE_DIR}/commandHandler.cpp
    ${PROJECT_SOURCE_DIR}/grpcServerStrategy.cpp
    ${PROJECT_SOURCE_DIR}/socketServerStrategy.cpp
    ${PROJECT_SOURCE_DIR}/softwareCore.cpp
    ${PROJECT_SOURCE_DIR}/main.cpp
)

# Create executables
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories for all targets
target_include_directories(${PROJECT_NAME} 
    PRIVATE 
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    Threads::Threads
    proto
)

# Platform-specific socket libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE 
        ws2_32
    )
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME}
        PRIVATE 
        /W4
    )
else()
    target_compile_options(${PROJECT_NAME}
        PRIVATE 
        -Wall -Wextra -Wpedantic
    )
endif()
