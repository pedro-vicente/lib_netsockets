#CMake build, Pedro Vicente 
#command line options for library dependencies
#-DOPENSSL_INCLUDE=PATH (e.g C:\ssl\include)
#-DOPENSSL_LIBRARY=FILE (e.g C:\ssl\lib\libssl.lib)
#-DCRYPTO_LIBRARY=FILE (e.g C:\ssl\lib\libcrypto.lib)

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
endif(MSVC)

include_directories(src)

set(SOURCES ${SOURCES} src/socket.hh)
set(SOURCES ${SOURCES} src/socket.cc)
set(SOURCES ${SOURCES} src/http.hh)
set(SOURCES ${SOURCES} src/http.cc)
set(SOURCES ${SOURCES} src/ftp.hh)
set(SOURCES ${SOURCES} src/ftp.cc)
set(SOURCES ${SOURCES} external/gason/src/gason.h)
set(SOURCES ${SOURCES} external/gason/src/gason.cpp)
set(SOURCES ${SOURCES} examples/json_message.hh)
set(SOURCES ${SOURCES} examples/json_message.cc)
set(SOURCES ${SOURCES} examples/sql_message.hh)
set(SOURCES ${SOURCES} examples/sql_message.cc)

add_executable(tcp_client examples/tcp_client.cc ${SOURCES})
add_executable(tcp_server examples/tcp_server.cc ${SOURCES})
add_executable(http_client examples/http_client.cc ${SOURCES})
add_executable(http_server examples/http_server.cc ${SOURCES})
add_executable(ftp_client examples/ftp_client.cc ${SOURCES})
add_executable(json_client examples/json_client.cc ${SOURCES})
add_executable(json_server examples/json_server.cc ${SOURCES})
target_include_directories(json_server PRIVATE external/gason/src)
add_executable(http_coin examples/http_coin.cc examples/coin.cc examples/coin.hh ${SOURCES})
target_include_directories(http_coin PRIVATE external/gason/src)

target_link_libraries (tcp_client ${LIBS})
target_link_libraries (tcp_server ${LIBS})
target_link_libraries (http_client ${LIBS})
target_link_libraries (http_server ${LIBS})
target_link_libraries (ftp_client ${LIBS})
target_link_libraries (json_client ${LIBS})
target_link_libraries (json_server ${LIBS})
target_link_libraries (http_coin ${LIBS})

#///////////////////////////////////////////////////////
#SQLite server/client example (uses JSON, SQLite)
#///////////////////////////////////////////////////////

set(SOURCES ${SOURCES} external/sqlite-3.18.0/sqlite3.h)
set(SOURCES ${SOURCES} external/sqlite-3.18.0/sqlite3.c)
add_executable(sqlite_server examples/sqlite_server.cc ${SOURCES})
add_executable(sqlite_client examples/sqlite_client.cc ${SOURCES})
target_include_directories(sqlite_server PRIVATE external/sqlite-3.18.0)
target_include_directories(sqlite_server PRIVATE external/gason/src)
target_link_libraries (sqlite_server ${LIBS} ${CMAKE_DL_LIBS})
target_include_directories(sqlite_client PRIVATE external/sqlite-3.18.0)
target_include_directories(sqlite_client PRIVATE external/gason/src)
target_link_libraries (sqlite_client ${LIBS} ${CMAKE_DL_LIBS})


#///////////////////////////////////////////////////////
#OpenSSL
#///////////////////////////////////////////////////////

if(OPENSSL_INCLUDE)
  if(EXISTS "${OPENSSL_INCLUDE}/openssl/ssl.h")
  else()
    message(FATAL_ERROR "${color_red}/openssl/ssl.h not found at ${OPENSSL_INCLUDE}${color_reset}")
  endif()
endif()

find_path(OPENSSL_INCLUDE openssl/ssl.h)
if(OPENSSL_INCLUDE)
  message("-- Found /openssl/ssl.h header file at: " ${OPENSSL_INCLUDE})
else()
  message("-- OpenSSL not found")
endif()

if (OPENSSL_LIBRARY)
  if(EXISTS "${OPENSSL_LIBRARY}")
  else()
    message(FATAL_ERROR "${color_red}Specified ${OPENSSL_LIBRARY} does not exist${color_reset}")
  endif()
endif()

if (OPENSSL_INCLUDE)

  find_library(OPENSSL_LIBRARY NAMES ssl libssl.lib)
  if(NOT OPENSSL_LIBRARY)
    message(FATAL_ERROR "${color_red}OpenSSL library not found${color_reset}")
  else()
    message("-- Found OpenSSL library at: " ${OPENSSL_LIBRARY})
  endif()

  if (CRYPTO_LIBRARY)
    if(EXISTS "${CRYPTO_LIBRARY}")
    else()
      message(FATAL_ERROR "${color_red}Specified ${CRYPTO_LIBRARY} does not exist${color_reset}")
    endif()
  endif()

  find_library(CRYPTO_LIBRARY NAMES crypto libcrypto.lib)
  if(NOT CRYPTO_LIBRARY)
    message(FATAL_ERROR "${color_red}CRYPTO library not found${color_reset}")
  else()
    message("-- Found CRYPTO library at: " ${CRYPTO_LIBRARY})
  endif()

  if(OPENSSL_INCLUDE AND OPENSSL_LIBRARY AND CRYPTO_LIBRARY)
    message("-- Building SSL TCP example")
    add_executable(openssl_client src/openssl_socket.cc src/openssl_socket.hh examples/openssl_client.cc )
    add_executable(https_coin src/openssl_socket.cc src/openssl_socket.hh examples/coin.cc examples/coin.hh examples/https_coin.cc external/gason/src/gason.cpp)
    target_include_directories(openssl_client PRIVATE ${OPENSSL_INCLUDE})
    target_include_directories(https_coin PRIVATE ${OPENSSL_INCLUDE} external/gason/src)
    if (MSVC)
      set(LIBS ${LIBS} crypt32.lib)
    else()
      set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pthread")
    endif(MSVC)
    target_link_libraries (openssl_client ${LIBS} ${OPENSSL_LIBRARY} ${CRYPTO_LIBRARY} ${CMAKE_DL_LIBS}) 
    target_link_libraries (https_coin ${LIBS} ${OPENSSL_LIBRARY} ${CRYPTO_LIBRARY} ${CMAKE_DL_LIBS}) 
  endif()

endif(OPENSSL_INCLUDE)