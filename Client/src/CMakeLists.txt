cmake_minimum_required(VERSION 3.10)
project(RTypeScenes)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(SDL2 REQUIRED)
find_library(ENGINE_LIB NAMES engine_core PATHS "${CMAKE_SOURCE_DIR}/../../EngineModule/build/" NO_DEFAULT_PATH)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/scenes/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../EngineModule/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../EngineModule/src/subsystems/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../EngineModule/src/subsystems/rendering/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/systems/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/components/ 
    ${SDL2_INCLUDE_DIRS}
)

function(add_scene_lib SCENE_NAME)
    add_library(${SCENE_NAME}_target SHARED ${ARGN})

    target_link_libraries(${SCENE_NAME}_target PRIVATE ${SDL2_LIBRARIES} ${ENGINE_LIB})

    string(TOLOWER ${SCENE_NAME} LOW_NAME)
    set_target_properties(${SCENE_NAME}_target PROPERTIES 
        PREFIX "" 
        OUTPUT_NAME "libscene_${LOW_NAME}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../scenes/"
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
add_scene_lib(RTypeScene    "scenes/RtypeScene.cpp" "../../Shared/systems/ForceCtrl.cpp" "../../Shared/systems/ChargedShoot.cpp" "../../Shared/systems/LevelSystem.cpp" "../../Shared/systems/Movement/Movement.cpp" "../../Shared/systems/PhysicsSystem.cpp" "../../Shared/systems/WeaponSystem.cpp" "../../Shared/systems/ProjectileSystem.cpp" "../../Shared/systems/Collision/Collision.cpp" "../../Shared/systems/BoundsSystem.cpp") 

add_scene_lib(MainMenu      "scenes/MainMenu.cpp")
add_scene_lib(Options       "scenes/Options.cpp")
add_scene_lib(WaitLobby     "scenes/WaitLobby.cpp")
