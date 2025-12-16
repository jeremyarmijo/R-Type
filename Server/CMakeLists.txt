cmake_minimum_required(VERSION 3.10)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)

project(RTYPE_Server VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/network/*.cpp
    ${PROJECT_SOURCE_DIR}/../Shared/network/*.cpp
    ${PROJECT_SOURCE_DIR}/../Shared/systems/*.cpp

)

add_executable(RTYPE_Server ${SOURCES})

target_include_directories(RTYPE_Server PRIVATE
    ${PROJECT_SOURCE_DIR}/../Shared
    ${PROJECT_SOURCE_DIR}/../Shared/components
    ${PROJECT_SOURCE_DIR}/include/network
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/../Client/src/engine/
    ${PROJECT_SOURCE_DIR}/../Shared/systems/
    ${PROJECT_SOURCE_DIR}/
)

find_package(asio REQUIRED)

target_link_libraries(RTYPE_Server PRIVATE asio::asio)

target_compile_options(RTYPE_Server PRIVATE -Wall -Wextra)
