cmake_minimum_required(VERSION 3.14)

project(RTYPE_Client VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)

find_package(SDL2 QUIET)
find_package(SDL2_image QUIET)
find_package(SDL2_ttf QUIET)
find_package(SDL2_mixer QUIET)

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
      "SDL2MIXER_CMD ON"
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

CPMAddPackage(
  NAME asio
  GITHUB_REPOSITORY chriskohlhoff/asio
  GIT_TAG asio-1-30-2
)

if(asio_ADDED)
  add_library(asio INTERFACE)
  target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
  target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
endif()

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

target_link_libraries(RTYPE_Client PRIVATE SDL2::SDL2 SDL2_image SDL2_ttf SDL2_mixer asio)

target_compile_options(RTYPE_Client PRIVATE -Wall -Wextra)
