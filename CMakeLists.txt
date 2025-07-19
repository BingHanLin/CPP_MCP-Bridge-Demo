cmake_minimum_required(VERSION 3.10)

project(cpp_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CUSTOMIZE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CUSTOMIZE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CUSTOMIZE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/archive")

if (MSVC) # only need for multi config build system
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        message(STATUS OUTPUTCONFIG....${OUTPUTCONFIG})
        string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CUSTOMIZE_RUNTIME_OUTPUT_DIRECTORY})
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CUSTOMIZE_LIBRARY_OUTPUT_DIRECTORY})
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CUSTOMIZE_ARCHIVE_OUTPUT_DIRECTORY})
    endforeach()
endif()

# Find required packages
find_package(Threads REQUIRED)

# Set source files
set(SOURCES
    ${PROJECT_SOURCE_DIR}/cpp_app/main.cpp
    ${PROJECT_SOURCE_DIR}/cpp_app/commandHandler.cpp
    ${PROJECT_SOURCE_DIR}/cpp_app/commandServer.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} 
    PRIVATE 
    ${PROJECT_SOURCE_DIR}/cpp_app
)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    Threads::Threads
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