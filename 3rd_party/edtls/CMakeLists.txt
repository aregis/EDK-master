cmake_minimum_required(VERSION 2.8)

project(edtls)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(cmake/GitInfo.cmake)
include(ExternalProject)
include(GenerateExportHeader)

if (WIN32)
    add_definitions(-DWIN32)
endif ()

generate_build_info(${CMAKE_CURRENT_SOURCE_DIR})

#============================================
# CONFIGURATION OPTIONS
#============================================
option(BUILD_DTLS_LIB "Build 3rdparty DTLS library" OFF)
option(BUILD_GTEST "Build google test" OFF)
option(BUILD_TEST "Build tests" OFF)
option(BUILD_CLIENT "Build DTLS Client" ON)
option(BUILD_SERVER "Build DTLS Server" ON)
option(BUILD_WITHOUT_RTTI "Build without RTTI support" ON)

if (((NOT BUILD_CLIENT) OR (NOT BUILD_SERVER)) AND BUILD_TEST)
    message(WARNING "Testing requires building both client and server. Turning off tests")
    set(BUILD_GTEST OFF)
    set(BUILD_TEST OFF)
endif ()

#============================================
# COMPILER DETECTION AND SETTINGS
#============================================
# RTTI support
if (BUILD_WITHOUT_RTTI)
    add_definitions(-DNO_RTTI)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    # ------------------------------------------
    # CLANG
    # ------------------------------------------
    add_definitions(-DCLANG)
    # RTTI support
    if (BUILD_WITHOUT_RTTI)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    endif ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # ------------------------------------------
    # GNU GCC
    # ------------------------------------------
    add_definitions(-DGNU)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)

    # Extra defines for detecting specific GCC version
    string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})
    list(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)
    set(GCC_MINOR "0")
    list(LENGTH GCC_VERSION_COMPONENTS listlength)
    if (listlength GREATER "1")
        list(GET GCC_VERSION_COMPONENTS 1 GCC_MINOR)
    endif()
    add_definitions(-DGCC_MAJOR=${GCC_MAJOR})
    add_definitions(-DGCC_MINOR=${GCC_MINOR})
    # RTTI support
    if (BUILD_WITHOUT_RTTI)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    endif ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lm")
    if (UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    endif ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-reorder")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # ------------------------------------------
    # Visual Studio
    # ------------------------------------------

    # RTTI support
    if (BUILD_WITHOUT_RTTI)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -GR-")
    endif ()

endif ()
find_package(Threads REQUIRED)

add_subdirectory(3rdparty)
add_subdirectory(libedtls)