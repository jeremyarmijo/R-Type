cmake_minimum_required(VERSION 3.14)
project(RTypeEngine VERSION 1.0 LANGUAGES CXX)

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
    "SDL_PIPEWIRE ON"
    "SDL_PIPEWIRE_SHARED ON"
    "SDL_PIPEWIRE_STATIC OFF"
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
    "SDL_PIPEWIRE ON"
    "SDL_PIPEWIRE_SHARED ON"
    "SDL_PIPEWIRE_STATIC OFF"
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

find_package(SDL2 QUIET)
find_package(SDL2_image QUIET)
find_package(SDL2_ttf QUIET)
find_package(SDL2_mixer QUIET)

add_library(engine_core SHARED
    src/engine/GameEngine.cpp
    src/scene/SceneManager.cpp
    src/scene/Scene.cpp
    src/subsystems/physics/Physics2D.hpp
    src/ecs/SparseArray.hpp
    src/ecs/Zipper.hpp
)

target_include_directories(engine_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src/engine/
    ${CMAKE_SOURCE_DIR}/src/
    ${CMAKE_SOURCE_DIR}/src/subsystems/
    ${CMAKE_SOURCE_DIR}/../Shared/
    ${CMAKE_SOURCE_DIR}/../Shared/components/
)

target_link_libraries(engine_core ${CMAKE_DL_LIBS} SDL2::SDL2 SDL2_image SDL2_ttf SDL2_mixer asio)

if(WIN32)
  target_link_libraries(engine_core ${CMAKE_DL_LIBS} ws2_32 wsock32 asio)
endif()


set_target_properties(engine_core PROPERTIES
    OUTPUT_NAME "engine_core"
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

if(WIN32)
    target_compile_definitions(engine_core PRIVATE ENGINE_EXPORTS)
endif()

add_library(subsystem_rendering SHARED
    src/subsystems/rendering/RenderingSubsystem.cpp
    
    # UI components
    src/subsystems/rendering/ui/UIElement.cpp
    src/subsystems/rendering/ui/UIButton.cpp
    src/subsystems/rendering/ui/UIText.cpp
    src/subsystems/rendering/ui/UITextInput.cpp
    src/subsystems/rendering/ui/UIImage.cpp
    src/subsystems/rendering/ui/UISolidColor.cpp
)

target_include_directories(subsystem_rendering PUBLIC
    ${CMAKE_SOURCE_DIR}/src/subsystems/rendering/
    ${CMAKE_SOURCE_DIR}/src/subsystems/
)

target_link_libraries(subsystem_rendering
    engine_core
    SDL2::SDL2
    SDL2_image
    SDL2_ttf
    SDL2_mixer
)

if(WIN32)
    target_compile_definitions(subsystem_rendering PRIVATE RENDERING_EXPORTS)
endif()

set_target_properties(subsystem_rendering PROPERTIES
    OUTPUT_NAME "subsystem_rendering"
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

add_library(subsystem_audio SHARED
    src/subsystems/audio/AudioSubsystem.cpp
)

target_include_directories(subsystem_audio PUBLIC
    ${CMAKE_SOURCE_DIR}/src/subsystems/
)

target_link_libraries(subsystem_audio
    engine_core
    SDL2_mixer::SDL2_mixer
)

if(WIN32)
    target_compile_definitions(subsystem_audio PRIVATE AUDIO_EXPORTS)
endif()

set_target_properties(subsystem_audio PROPERTIES
    OUTPUT_NAME "subsystem_audio"
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

add_library(subsystem_physics SHARED
    src/subsystems/physics/PhysicsSystem.cpp
)

target_include_directories(subsystem_physics PUBLIC
    ${CMAKE_SOURCE_DIR}/src/subsystems/
)

target_link_libraries(subsystem_physics
    engine_core
)

set_target_properties(subsystem_physics PROPERTIES
    OUTPUT_NAME "subsystem_physics"
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

add_library(subsystem_input SHARED
    src/subsystems/input/InputSubsystem.cpp
)

target_include_directories(subsystem_input PUBLIC
    ${CMAKE_SOURCE_DIR}/src/subsystems/
)

target_link_libraries(subsystem_input
    engine_core
    SDL2::SDL2
)

if(WIN32)
    target_compile_definitions(subsystem_input PRIVATE INPUT_EXPORTS)
endif()

set_target_properties(subsystem_input PROPERTIES
    OUTPUT_NAME "subsystem_input"
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

add_library(subsystem_network SHARED
    src/subsystems/network/NetworkSubsystem.cpp
    ../Shared/network/DecodeFunc.cpp
    ../Shared/network/Decoder.cpp
    ../Shared/network/Encode.cpp
    ../Shared/network/EncodeFunc.cpp
)

target_include_directories(subsystem_network PUBLIC
    ${CMAKE_SOURCE_DIR}/src/subsystems/
    ${CMAKE_SOURCE_DIR}/../Shared/
)

if(WIN32)
    target_compile_definitions(subsystem_network PRIVATE NETWORK_EXPORTS)
endif()

target_link_libraries(subsystem_network
    engine_core
    SDL2::SDL2
)

set_target_properties(subsystem_network PROPERTIES
    OUTPUT_NAME "subsystem_network"
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)
