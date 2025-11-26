cmake_minimum_required(VERSION 3.10)

project(RTYPE_Client VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_executable(RTYPE_Client ${SOURCES})

target_include_directories(RTYPE_Client PRIVATE
    ${PROJECT_SOURCE_DIR}/src/
    ${PROJECT_SOURCE_DIR}/src/engine/
    ${PROJECT_SOURCE_DIR}/src/engine/ecs/
    ${PROJECT_SOURCE_DIR}/src/engine/graphics/
    ${PROJECT_SOURCE_DIR}/src/engine/physics/
    ${PROJECT_SOURCE_DIR}/src/engine/inputs/
    ${PROJECT_SOURCE_DIR}/src/engine/network/
    ${PROJECT_SOURCE_DIR}/src/engine/scene/
)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
target_link_libraries(RTYPE_Client PRIVATE SDL2::SDL2 SDL2_image::SDL2_image)

target_compile_options(RTYPE_Client PRIVATE -Wall -Wextra)
