cmake_minimum_required(VERSION 3.25)
project(rpc)

set(CMAKE_CXX_STANDARD 20)

include_directories(${PROJECT_SOURCE_DIR}/rpc/include)
include_directories(${PROJECT_SOURCE_DIR}/rpc/include/common)
include_directories(${PROJECT_SOURCE_DIR}/rpc/include/common/log)
include_directories(${PROJECT_SOURCE_DIR}/rpc/include/net/coder)
include_directories(${PROJECT_SOURCE_DIR}/rpc/include/net/rpc)
include_directories(${PROJECT_SOURCE_DIR}/rpc/include/net/tcp)
include_directories(${PROJECT_SOURCE_DIR}/rpc/include/net)

# Create the rpc library
add_library(rpc "")

add_subdirectory(rpc/test)
add_subdirectory(rpc/src)

