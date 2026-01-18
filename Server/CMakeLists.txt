cmake_minimum_required(VERSION 3.14)

project(RTYPE_Server VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)

if(NOT SDL2_FOUND)
  CPMAddPackage(
    NAME SDL2
    GITHUB_REPOSITORY libsdl-org/SDL
    GIT_TAG release-2.28.5
    OPTIONS
      "SDL2_DISABLE_INSTALL OFF"
      "SDL_SHARED ON"
      "SDL_STATIC OFF"
      "SDL_PULSEAUDIO ON"
      "SDL_ALSA ON"
  )
endif()

if(NOT SDL2_image_FOUND)
  CPMAddPackage(
    NAME SDL2_image
    GITHUB_REPOSITORY libsdl-org/SDL_image
    GIT_TAG release-2.8.2
    OPTIONS
      "SDL2IMAGE_INSTALL ON"
  )
endif()

if(NOT SDL2_ttf_FOUND)
  CPMAddPackage(
    NAME SDL2_ttf
    GITHUB_REPOSITORY libsdl-org/SDL_ttf
    GIT_TAG release-2.22.0
    OPTIONS
      "SDL2TTF_INSTALL ON"
  )
endif()

if(NOT SDL2_mixer_FOUND)
  CPMAddPackage(
    NAME SDL2_mixer
    GITHUB_REPOSITORY libsdl-org/SDL_mixer
    GIT_TAG release-2.8.0
    OPTIONS
      "SDL2MIXER_INSTALL ON"
      "SDL2MIXER_CMD OFF"
      "SDL2MIXER_FLAC DRFLAC"
      "SDL2MIXER_MOD OFF"
      "SDL2MIXER_MP3 MINIMP3"
      "SDL2MIXER_MIDI OFF"
      "SDL2MIXER_VORBIS STB"
      "SDL2MIXER_OPUS OFF"
      "SDL2MIXER_WAVPACK OFF"
      "SDL2MIXER_GME OFF"
      "SDL2MIXER_WAV ON"
  )
endif()

if(NOT asio_FOUND)
CPMAddPackage(
  NAME asio
  GITHUB_REPOSITORY chriskohlhoff/asio
  GIT_TAG asio-1-30-2
)
endif()

if(asio_ADDED)
  add_library(asio INTERFACE)
  target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
  target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
endif()

file(GLOB SOURCES_MAIN
    ${PROJECT_SOURCE_DIR}/src/ServerGame.cpp
    ${PROJECT_SOURCE_DIR}/src/main.cpp
)

file(GLOB_RECURSE SOURCES_SHARED
    ${PROJECT_SOURCE_DIR}/../Shared/network/*.cpp
    ${PROJECT_SOURCE_DIR}/../Shared/systems/*.cpp
)

set(SOURCES ${SOURCES_MAIN} ${SOURCES_SHARED})

add_executable(RTYPE_Server ${SOURCES})

find_package(SDL2 REQUIRED)
find_library(ENGINE_LIB  NAMES engine_core         PATHS "${CMAKE_SOURCE_DIR}/../EngineModule/build/" NO_DEFAULT_PATH)
find_library(RENDER_LIB  NAMES subsystem_rendering PATHS "${CMAKE_SOURCE_DIR}/../EngineModule/build/" NO_DEFAULT_PATH)
find_library(AUDIO_LIB   NAMES subsystem_audio     PATHS "${CMAKE_SOURCE_DIR}/../EngineModule/build/" NO_DEFAULT_PATH)
find_library(INPUT_LIB   NAMES subsystem_input     PATHS "${CMAKE_SOURCE_DIR}/../EngineModule/build/" NO_DEFAULT_PATH)
find_library(NETWORK_LIB NAMES subsystem_network   PATHS "${CMAKE_SOURCE_DIR}/../EngineModule/build/" NO_DEFAULT_PATH)

target_include_directories(RTYPE_Server PRIVATE
    ${PROJECT_SOURCE_DIR}/../Shared/
    ${PROJECT_SOURCE_DIR}/../Shared/components/
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/../Shared/systems/
    ${PROJECT_SOURCE_DIR}/../Shared/dynamicLibLoader/
    ${PROJECT_SOURCE_DIR}/
    ${PROJECT_SOURCE_DIR}/../EngineModule/src/subsystems/
    ${PROJECT_SOURCE_DIR}/../EngineModule/src/
    ${PROJECT_SOURCE_DIR}/src/network/
)

target_link_libraries(RTYPE_Server PRIVATE ${ENGINE_LIB} ${RENDER_LIB} ${AUDIO_LIB} ${INPUT_LIB} ${NETWORK_LIB} SDL2::SDL2 SDL2_image SDL2_ttf SDL2_mixer asio)

if(WIN32)
  target_link_libraries(RTYPE_Server PRIVATE ${ENGINE_LIB} ${RENDER_LIB} ${AUDIO_LIB} ${INPUT_LIB} ${NETWORK_LIB} SDL2::SDL2main ws2_32 wsock32 asio)
endif()

target_compile_options(RTYPE_Server PRIVATE -Wall -Wextra)
