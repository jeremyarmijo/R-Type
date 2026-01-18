cmake_minimum_required(VERSION 3.14)
project(RTypeScenes)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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

if(NOT SDL2_ttf_FOUND)
  CPMAddPackage(
    NAME SDL2_ttf
    GITHUB_REPOSITORY libsdl-org/SDL_ttf
    GIT_TAG release-2.22.0
    OPTIONS
      "SDL2TTF_INSTALL ON"
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

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(asio REQUIRED)
find_library(ENGINE_LIB  NAMES engine_core libengine_core         PATHS "${CMAKE_SOURCE_DIR}/../../EngineModule/build/" NO_DEFAULT_PATH)
find_library(RENDER_LIB  NAMES subsystem_rendering libsubsystem_rendering PATHS "${CMAKE_SOURCE_DIR}/../../EngineModule/build/" NO_DEFAULT_PATH)
find_library(AUDIO_LIB   NAMES subsystem_audio libsubsystem_audio     PATHS "${CMAKE_SOURCE_DIR}/../../EngineModule/build/" NO_DEFAULT_PATH)
find_library(INPUT_LIB   NAMES subsystem_input libsubsystem_input     PATHS "${CMAKE_SOURCE_DIR}/../../EngineModule/build/" NO_DEFAULT_PATH)
find_library(NETWORK_LIB NAMES subsystem_network libsubsystem_network   PATHS "${CMAKE_SOURCE_DIR}/../../EngineModule/build/" NO_DEFAULT_PATH)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/scenes/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../EngineModule/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../EngineModule/src/subsystems/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../EngineModule/src/subsystems/rendering/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/systems/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/components/ 
)

function(add_scene_lib SCENE_NAME)
    add_library(${SCENE_NAME}_target SHARED ${ARGN})

    target_link_libraries(${SCENE_NAME}_target PRIVATE SDL2::SDL2 SDL2_mixer SDL2_image SDL2_ttf asio ${RENDER_LIB} ${AUDIO_LIB} ${INPUT_LIB} ${NETWORK_LIB} ${ENGINE_LIB})

    if(WIN32)
        target_link_libraries(${SCENE_NAME}_target PRIVATE SDL2::SDL2main ws2_32 wsock32)
    endif()

    string(TOLOWER ${SCENE_NAME} LOW_NAME)
    set_target_properties(${SCENE_NAME}_target PROPERTIES
        PREFIX ""
        OUTPUT_NAME "libscene_${LOW_NAME}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../scenes/"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../scenes/"
    )
endfunction()


add_scene_lib(CreateLobby   "scenes/CreateLobby.cpp")
add_scene_lib(GameOver      "scenes/GameOver.cpp")
add_scene_lib(JoinGame      "scenes/JoinGame.cpp")
add_scene_lib(Lobby         "scenes/Lobby.cpp")
add_scene_lib(LobbyInfo     "scenes/LobbyInfo.cpp")
add_scene_lib(LobbyJoin     "scenes/LobbyJoin.cpp")
add_scene_lib(LobbyPassword "scenes/LobbyPassword.cpp")

add_scene_lib(MainGame      "scenes/MainGame.cpp" "../../Shared/systems/InputSystem.cpp" "../../Shared/systems/LevelSystem.cpp" "../../Shared/systems/Movement/Movement.cpp" "../../Shared/systems/PhysicsSystem.cpp" "../../Shared/systems/WeaponSystem.cpp" "../../Shared/systems/ProjectileSystem.cpp" "../../Shared/systems/Collision/Collision.cpp" "../../Shared/systems/BoundsSystem.cpp")
add_scene_lib(RTypeScene    "scenes/RtypeScene.cpp" "../../Shared/systems/LevelSystem.cpp" "../../Shared/systems/Movement/Movement.cpp" "../../Shared/systems/PhysicsSystem.cpp" "../../Shared/systems/WeaponSystem.cpp" "../../Shared/systems/ProjectileSystem.cpp" "../../Shared/systems/Collision/Collision.cpp" "../../Shared/systems/BoundsSystem.cpp") 

add_scene_lib(MainMenu      "scenes/MainMenu.cpp")
add_scene_lib(Options       "scenes/Options.cpp")
add_scene_lib(WaitLobby     "scenes/WaitLobby.cpp")
