project(proto)

include(../cmake/common.cmake)

# Protobuf/Grpc source files
#
file(GLOB_RECURSE PROTO_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/mcp_service.proto")

#
# Add Library target with protobuf sources
#
add_library(${PROJECT_NAME} ${PROTO_FILES})

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

#
# Compile protobuf and grpc files in myproto target to cpp
#
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET ${PROJECT_NAME} LANGUAGE cpp)
protobuf_generate(TARGET ${PROJECT_NAME} LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")
