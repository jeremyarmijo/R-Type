cmake_minimum_required(VERSION 3.14)

project(RTYPE_Server VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME asio
  GITHUB_REPOSITORY chriskohlhoff/asio
  GIT_TAG asio-1-30-2
  OPTIONS
    "ASIO_STANDALONE ON"
)

if(asio_ADDED)
  add_library(asio INTERFACE)
  target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
  target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
endif()

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/network/*.cpp
    ${PROJECT_SOURCE_DIR}/../../Shared/network/*.cpp
)

add_library(network_server SHARED ${SOURCES})

target_include_directories(network_server PRIVATE
    ${PROJECT_SOURCE_DIR}/../../Shared
    ${PROJECT_SOURCE_DIR}/../include/network
    ${PROJECT_SOURCE_DIR}/../include/
    ${PROJECT_SOURCE_DIR}/../
)

target_link_libraries(network_server PRIVATE asio)

target_compile_options(network_server PRIVATE -Wall -Wextra)
