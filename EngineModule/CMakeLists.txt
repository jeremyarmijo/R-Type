cmake_minimum_required(VERSION 3.10)
project(RTypeEngine VERSION 1.0.0)

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

target_link_libraries(engine_core
    ${CMAKE_DL_LIBS}
    SDL2::SDL2
)

set_target_properties(engine_core PROPERTIES
    OUTPUT_NAME "engine_core"
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Export symbols on Windows
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
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
)

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

target_link_libraries(subsystem_network
    engine_core
    SDL2::SDL2
)

set_target_properties(subsystem_network PROPERTIES
    OUTPUT_NAME "subsystem_network"
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)
