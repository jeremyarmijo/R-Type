cmake_minimum_required(VERSION 3.10)
project(RTypeScenes)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Dependencies
find_package(SDL2 REQUIRED)
find_library(ENGINE_LIB  NAMES engine_core PATHS "${CMAKE_SOURCE_DIR}/../../EngineModule/build/" NO_DEFAULT_PATH)

# 2. Include Paths
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../EngineModule/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../EngineModule/src/subsystems/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../EngineModule/src/subsystems/rendering/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/systems/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/components/
    ${SDL2_INCLUDE_DIRS}
)

file(GLOB SCENE_SOURCES "scenes/*.cpp")

foreach(SOURCE_FILE ${SCENE_SOURCES})
    get_filename_component(BASE_NAME ${SOURCE_FILE} NAME_WE)
    
    string(TOLOWER ${BASE_NAME} LOW_NAME)

    add_library(${BASE_NAME}_target SHARED ${SOURCE_FILE})

    target_link_libraries(${BASE_NAME}_target PRIVATE 
        ${SDL2_LIBRARIES}
        ${ENGINE_LIB} 
    )

    set_target_properties(${BASE_NAME}_target PROPERTIES 
        PREFIX "" 
        OUTPUT_NAME "libscene_${LOW_NAME}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../scenes/"
    )
endforeach()
