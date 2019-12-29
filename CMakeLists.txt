#CMake build, Pedro Vicente 
#command line options for library dependencies

cmake_minimum_required(VERSION 3.1)
project (lib_netsockets)
include(CheckCXXCompilerFlag)
set(BUILD_SHARED_LIBRARIES OFF)
set (CMAKE_CXX_STANDARD 11)

if (MSVC)
  add_definitions( "/D_CRT_SECURE_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS" )
endif(MSVC)

set(LIBS ${LIBS})
if (MSVC)
  set(LIBS ${LIBS} ws2_32.lib)
else()
  set( CMAKE_CXX_FLAGS " -pthread " )
endif(MSVC)

include_directories(src)

set(sources ${sources})
set(sources ${sources} src/socket.hh)
set(sources ${sources} src/socket.cc)
set(sources ${sources} src/http.hh)
set(sources ${sources} src/http.cc)
set(sources ${sources} src/ftp.hh)
set(sources ${sources} src/ftp.cc)

add_library(lib_netsockets ${sources})
add_executable(pub_sub_client examples/pub_sub_client.cc)
add_executable(pub_sub_server examples/pub_sub_server.cc)
add_executable(block_client examples/block_client.cc)
add_executable(block_server examples/block_server.cc)
add_executable(select_server examples/select_server.cc)
add_executable(http_client examples/http_client.cc)
add_executable(http_server examples/http_server.cc)
add_executable(ftp_client examples/ftp_client.cc)
add_executable(json_client examples/json_client.cc examples/json_message.hh examples/json_message.cc external/gason/src/gason.cpp)
add_executable(json_server examples/json_server.cc examples/json_message.hh examples/json_message.cc external/gason/src/gason.cpp)
target_include_directories(json_server PRIVATE external/gason/src)

set(lib_netsockets lib_netsockets)
target_link_libraries (pub_sub_client ${lib_netsockets} ${LIBS})
target_link_libraries (pub_sub_server ${lib_netsockets} ${LIBS})
target_link_libraries (block_client ${lib_netsockets} ${LIBS})
target_link_libraries (block_server ${lib_netsockets} ${LIBS})
target_link_libraries (select_server ${lib_netsockets} ${LIBS})
target_link_libraries (http_client ${lib_netsockets} ${LIBS})
target_link_libraries (http_server ${lib_netsockets} ${LIBS})
target_link_libraries (ftp_client ${lib_netsockets} ${LIBS})
target_link_libraries (json_client ${lib_netsockets} ${LIBS})
target_link_libraries (json_server ${lib_netsockets} ${LIBS})

#///////////////////////////////////////////////////////
#SQLite server/client example (uses JSON, SQLite)
#///////////////////////////////////////////////////////

add_executable(sqlite_server examples/sqlite_server.cc external/sqlite-3.18.0/sqlite3.c external/gason/src/gason.cpp)
add_executable(sqlite_client examples/sqlite_client.cc examples/sql_message.hh examples/sql_message.cc external/sqlite-3.18.0/sqlite3.c external/gason/src/gason.cpp)
target_include_directories(sqlite_server PRIVATE external/sqlite-3.18.0)
target_include_directories(sqlite_server PRIVATE external/gason/src)
target_link_libraries (sqlite_server ${lib_netsockets} ${LIBS} ${CMAKE_DL_LIBS})
target_include_directories(sqlite_client PRIVATE external/sqlite-3.18.0)
target_include_directories(sqlite_client PRIVATE external/gason/src)
target_link_libraries (sqlite_client ${lib_netsockets} ${LIBS} ${CMAKE_DL_LIBS})
