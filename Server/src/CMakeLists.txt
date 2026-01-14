cmake_minimum_required(VERSION 3.14)

project(RTYPE_Server VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME asio
  GITHUB_REPOSITORY chriskohlhoff/asio
  GIT_TAG asio-1-30-2
  OPTIONS
    "ASIO_STANDALONE ON"
)

CPMAddPackage(
  NAME sqlite3
  URL https://www.sqlite.org/2024/sqlite-amalgamation-3450100.zip
  DOWNLOAD_ONLY YES
)

if(asio_ADDED)
  add_library(asio INTERFACE)
  target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
  target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
endif()

if(sqlite3_ADDED)
  add_library(sqlite3 STATIC ${sqlite3_SOURCE_DIR}/sqlite3.c)
  target_include_directories(sqlite3 PUBLIC ${sqlite3_SOURCE_DIR})
  target_compile_definitions(sqlite3 PUBLIC SQLITE_THREADSAFE=1)
  set_target_properties(sqlite3 PROPERTIES 
    LINKER_LANGUAGE C
    POSITION_INDEPENDENT_CODE ON
  )
  
  if(WIN32)
    target_compile_definitions(sqlite3 PRIVATE SQLITE_API=__declspec(dllexport))
  endif()
  
  if(UNIX)
    target_link_libraries(sqlite3 PUBLIC pthread dl)
  endif()
endif()

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/network/*.cpp
    ${PROJECT_SOURCE_DIR}/../../Shared/network/*.cpp
)

add_library(network_server SHARED ${SOURCES})

target_include_directories(network_server PRIVATE
    ${PROJECT_SOURCE_DIR}/../../Shared
    ${PROJECT_SOURCE_DIR}/../include/network
    ${PROJECT_SOURCE_DIR}/../include/
    ${PROJECT_SOURCE_DIR}/../
)

target_link_libraries(network_server PRIVATE asio sqlite3)

if(WIN32)
  target_link_libraries(network_server PRIVATE ws2_32 wsock32)
endif()

target_compile_options(network_server PRIVATE -Wall -Wextra)
