#============================================
#  ______  _____   _  __
# |  ____||  __ \ | |/ /
# | |__   | |  | || ' /
# |  __|  | |  | ||  <
# | |____ | |__| || . \
# |______||_____/ |_|\_\
#                                 Philips HUE
#============================================
cmake_minimum_required (VERSION 3.2)
cmake_policy(SET CMP0054 NEW)

project (libhuestream C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
#============================================
# cmake modules
#============================================
set(CMAKE_MODULE_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake
        ${CMAKE_MODULE_PATH}
        )

#============================================
# Install dirs
#============================================

set(BUILD_DIR ${CMAKE_BINARY_DIR}/bin)
file(MAKE_DIRECTORY ${BUILD_DIR})

if (NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
endif()

set(INSTALL_INC_DIR_NAME include)
set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BUILD_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BUILD_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${BUILD_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${BUILD_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${BUILD_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${BUILD_DIR}")
#============================================
# External projects and cmake modules
#============================================
include(Common)
include(GitInfo)
include(ExternalProject)
include(ExternalData)
include(GenerateExportHeader)
include(Helpers)
include(PostInstall)

#============================================
# ctest
#============================================
enable_testing()
set(CTEST_OUTPUT_ON_FAILURE 1)

generate_build_info(${CMAKE_SOURCE_DIR}/huesdk)
#============================================
# CONFIGURATION OPTIONS
#============================================
option(BUILD_CURL "Build Curl" ON)
option(BUILD_TEST "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_TOOLS "Build tools" ON)
option(BUILD_SWIG "Build swig" OFF)
option(BUILD_WRAPPERS "Build wrappers" OFF)
option(BUILD_TEST_COV "Build tests with coverage" OFF)
option(BUILD_WITH_RTTI "Build with RTTI support" ON)
option(BUILD_WITH_EXCEPTIONS "Build with exception support" ON)
option(ENABLE_JNI "Enable JNI" ${BUILD_WRAPPERS})

#============================================
# SHARED/STATIC settings
#============================================
if (NOT LIB_BUILD_MODE)
    set(LIB_BUILD_MODE "STATIC")
endif()

if (LIB_BUILD_MODE STREQUAL "SHARED")
    set(BUILD_SHARED ON)
elseif(LIB_BUILD_MODE STREQUAL "STATIC")
    set(BUILD_STATIC ON)
else()
    message(FATAL_ERROR "LIB_BUILD_MODE has invalid value: ${LIB_BUILD_MODE}")
endif()

#============================================
# COMPILER DETECTION AND SETTINGS
#============================================
if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(WIN32)
    add_definitions(-DWIN32)
endif()

# RTTI support
if (NOT BUILD_WITH_RTTI)
    add_definitions(-DNO_RTTI)
endif()

if (NOT BUILD_WITH_EXCEPTIONS)
    add_definitions(-D_HAS_EXCEPTIONS=0)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    # ------------------------------------------
    # CLANG
    # ------------------------------------------

    # Coverage
    if (BUILD_TEST_COV)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    endif()

    # RTTI support
    if (NOT BUILD_WITH_RTTI)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    endif()

    # Exception support
    if (NOT BUILD_WITH_EXCEPTIONS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
    endif()

    if (UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    endif (UNIX)

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # ------------------------------------------
    # GNU GCC
    # ------------------------------------------

    # RTTI support
    if (NOT BUILD_WITH_RTTI)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    endif()

    # Exception support
    if (NOT BUILD_WITH_EXCEPTIONS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++${CMAKE_CXX_STANDARD}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lm")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-reorder")

    if (UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    endif (UNIX)

    # Coverage
    if (BUILD_TEST_COV)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage -fprofile-arcs -ftest-coverage")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # ------------------------------------------
    # Visual Studio
    # ------------------------------------------
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)

    # Parallel build
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

    # RTTI support
    if (NOT BUILD_WITH_RTTI)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -GR-")
    endif()

    # Exception support
    if (NOT BUILD_WITH_EXCEPTIONS)
        string(REGEX REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()

    # Specific code define for exporting DLLs
    if (BUILD_SHARED)
        add_definitions(-DUSE_EXPORT)
    endif()
endif()

#In this variable the shared libraries needed are collected and on windows
# copied to output next the executable
set(shared_libs "")

#============================================
# 3rd_party
#============================================
# Setup download information for 3rd party libraries
include(SetupThirdParty)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/CMakeLists.txt)
    add_subdirectory(3rd_party)
endif()

#============================================
# libhuestream
#============================================
include_directories(libhuestream)
add_subdirectory(libhuestream)

#============================================
# examples
#============================================
if (BUILD_EXAMPLES)
    if (XCODE)
        message("EDK Examples disabled because of lack of support for console apps on mobile platforms")
    else(XCODE)
        add_subdirectory(examples)
    endif(XCODE)
endif()

if (BUILD_TOOLS)
    if (XCODE)
        message("EDK Tools disabled because of lack of support for console apps on mobile platforms")
    else(XCODE)
        add_subdirectory(tools)
    endif(XCODE)
endif()

#============================================
# wrappers
#============================================
if (BUILD_WRAPPERS)
    add_subdirectory(wrappers)
endif()

add_custom_target(install_huestream
        ${MAKE} install
        DEPENDS huestream
        COMMENT "Installing huestream")
