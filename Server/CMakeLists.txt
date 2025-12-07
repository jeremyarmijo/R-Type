cmake_minimum_required(VERSION 3.10)

project(RTYPE_Client VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/network/*.cpp
)

add_executable(RTYPE_Client ${SOURCES})

target_include_directories(RTYPE_Client PRIVATE
    ${PROJECT_SOURCE_DIR}/include/network
    ${PROJECT_SOURCE_DIR}/include/
)

# find_package(SDL2 REQUIRED)
# find_package(SDL2_image REQUIRED)
# target_link_libraries(RTYPE_Client PRIVATE SDL2::SDL2 SDL2_image::SDL2_image)

target_compile_options(RTYPE_Client PRIVATE -Wall -Wextra)
