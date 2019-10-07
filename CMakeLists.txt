#CMake build, Pedro Vicente 
#command line options for library dependencies

cmake_minimum_required(VERSION 2.8)
project (lib_netsockets)
include(CheckCXXCompilerFlag)
set(BUILD_SHARED_LIBRARIES OFF)

#//////////////////////////
# colorized output
# red, fatal error, cannot build (missing netCDF, HDF5, CURL libraries)
# magenta, "negative" warning (missing optional libraries, GSL, ANTLR, UDUNITS)
# green, "positive" warning (opposite of magenta, optional libraries found)
# blue, general information to take notice (SZIP/ZLIB were detected as needed, special NCO functions are defined)
#//////////////////////////

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(color_reset   "${Esc}[m")
  set(color_magenta "${Esc}[35m")
  set(color_red     "${Esc}[31m")
  set(color_green   "${Esc}[32m")
  set(color_blue    "${Esc}[34m")
endif()

#///////////////////////////////////////////////////////
#MSVC
#///////////////////////////////////////////////////////

if (MSVC)
  add_definitions( "/D_CRT_SECURE_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS" )
  if (STATIC_CRT)
    message("-- Building with static runtime library")
    set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1")
    set(CMAKE_CXX_FLAGS_MINSIZEREL     "/MT /O1 /Ob1 /D NDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE        "/MT /O2 /Ob2 /D NDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT /Zi /O2 /Ob1 /D NDEBUG")
  endif()  
endif(MSVC)

if (CMAKE_VERSION VERSION_LESS "3.1")
 if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
   CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
   if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
   elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
   else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
   endif()
 endif()
else ()
  set (CMAKE_CXX_STANDARD 11)
endif()

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
add_executable(tcp_client examples/tcp_client.cc)
add_executable(tcp_server examples/tcp_server.cc)
add_executable(http_client examples/http_client.cc)
add_executable(http_server examples/http_server.cc)
add_executable(ftp_client examples/ftp_client.cc)
add_executable(json_client examples/json_client.cc examples/json_message.hh examples/json_message.cc external/gason/src/gason.cpp)
add_executable(json_server examples/json_server.cc examples/json_message.hh examples/json_message.cc external/gason/src/gason.cpp)
target_include_directories(json_server PRIVATE external/gason/src)
add_executable(http_coin examples/http_coin.cc examples/coin.cc examples/coin.hh examples/json_message.cc external/gason/src/gason.cpp)
target_include_directories(http_coin PRIVATE external/gason/src)

set(lib_netsockets lib_netsockets)
target_link_libraries (tcp_client ${lib_netsockets} ${LIBS})
target_link_libraries (tcp_server ${lib_netsockets} ${LIBS})
target_link_libraries (http_client ${lib_netsockets} ${LIBS})
target_link_libraries (http_server ${lib_netsockets} ${LIBS})
target_link_libraries (ftp_client ${lib_netsockets} ${LIBS})
target_link_libraries (json_client ${lib_netsockets} ${LIBS})
target_link_libraries (json_server ${lib_netsockets} ${LIBS})
target_link_libraries (http_coin ${lib_netsockets} ${LIBS})

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
