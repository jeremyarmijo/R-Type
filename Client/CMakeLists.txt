cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(RTYPE_Client VERSION 1.0 LANGUAGES CXX)


add_executable(RTYPE_Client src/Main.cpp)

target_include_directories(RTYPE_Client PRIVATE
    ${CMAKE_SOURCE_DIR}/../EngineModule/src/
    ${CMAKE_SOURCE_DIR}/../EngineModule/src/subsystems
    ${CMAKE_SOURCE_DIR}/../Shared/
)

add_custom_target(clean-all
    COMMAND rm -rf CMakeFiles CMakeCache.txt cmake_install.cmake Makefile
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

find_library(ENGINE_LIB  NAMES engine_core         PATHS "${CMAKE_SOURCE_DIR}/../EngineModule/build/" NO_DEFAULT_PATH)
find_library(RENDER_LIB  NAMES subsystem_rendering PATHS "${CMAKE_SOURCE_DIR}/../EngineModule/build/" NO_DEFAULT_PATH)
find_library(AUDIO_LIB   NAMES subsystem_audio     PATHS "${CMAKE_SOURCE_DIR}/../EngineModule/build/" NO_DEFAULT_PATH)
find_library(INPUT_LIB   NAMES subsystem_input     PATHS "${CMAKE_SOURCE_DIR}/../EngineModule/build/" NO_DEFAULT_PATH)
find_library(PHYSICS_LIB NAMES subsystem_physics   PATHS "${CMAKE_SOURCE_DIR}/../EngineModule/build/" NO_DEFAULT_PATH)
find_library(NETWORK_LIB NAMES subsystem_network   PATHS "${CMAKE_SOURCE_DIR}/../EngineModule/build/" NO_DEFAULT_PATH)

if(NOT ENGINE_LIB)
    message(FATAL_ERROR "Could not find libengine_core.so in ../EngineModule/build/")
endif()


target_link_libraries(RTYPE_Client PRIVATE ${ENGINE_LIB} ${RENDER_LIB} ${AUDIO_LIB} ${INPUT_LIB} ${PHYSICS_LIB} ${NETWORK_LIB} SDL2::SDL2 SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer)

set_target_properties(RTYPE_Client PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "${CMAKE_SOURCE_DIR}/../EngineModule/build/"
)

target_compile_options(RTYPE_Client PRIVATE -Wall -Wextra)
