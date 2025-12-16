cmake_minimum_required(VERSION 3.10)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)

project(RTYPE_Client VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/Client/src/*.cpp
    ${PROJECT_SOURCE_DIR}/Shared/*.cpp
)

add_executable(RTYPE_Client ${SOURCES})

target_include_directories(RTYPE_Client PRIVATE
    ${PROJECT_SOURCE_DIR}/Shared/
    ${PROJECT_SOURCE_DIR}/Shared/ecs/
    ${PROJECT_SOURCE_DIR}/Shared/components/
    ${PROJECT_SOURCE_DIR}/Shared/network/
    ${PROJECT_SOURCE_DIR}/Shared/systems/
    ${PROJECT_SOURCE_DIR}/Client/src/
    ${PROJECT_SOURCE_DIR}/Client/src/scenes/
    ${PROJECT_SOURCE_DIR}/Client/src/engine/
    ${PROJECT_SOURCE_DIR}/Client/src/engine/ui/
    ${PROJECT_SOURCE_DIR}/Client/src/engine/graphics/
    ${PROJECT_SOURCE_DIR}/Client/src/engine/inputs/
    ${PROJECT_SOURCE_DIR}/Client/src/engine/network/
    ${PROJECT_SOURCE_DIR}/Client/src/engine/scene/
    ${PROJECT_SOURCE_DIR}/Client/src/engine/settings/
)

add_custom_target(clean-all
    COMMAND rm -rf CMakeFiles CMakeCache.txt cmake_install.cmake Makefile
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

find_package(asio REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)
target_link_libraries(RTYPE_Client PRIVATE SDL2::SDL2 SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer)

target_compile_options(RTYPE_Client PRIVATE -Wall -Wextra)
