cmake_minimum_required(VERSION 3.6)

project(support)

#
# support sources
#
file(GLOB_RECURSE SUPPORT_SOURCE_FILES
        "src/crypto/*.cpp" "include/support/crypto/*.h"
        "src/chrono/*.cpp" "include/support/chrono/*.h"
        "src/std/*.cpp" "include/support/std/*.h"
        "src/date/*.cpp" "include/support/date/*.h"
        "src/chrono/*.cpp" "include/support/chrono/*.h"
        "src/logging/*.cpp" "include/support/logging/*.h"
        "src/threading/*.cpp" "include/support/threading/*.h"
        "src/network/*.cpp" "include/support/network/*.h"
        "src/util/*.cpp" "include/support/util/*.h"
        "src/scheduler/*.cpp" "include/support/scheduler/*.h"
        "src/persistence/*.cpp" "include/support/persistence/*.h"
        "src/os/*.cpp" "include/support/os/*.h"
)

list(APPEND SUPPORT_SOURCES ${SUPPORT_SOURCE_FILES})

#
# android dependencies
#
if(ANDROID)
    find_library(ANDROID_LOG_LIB log)
    list(APPEND SUPPORT_DEFINITIONS_PUBLIC ANDROID ANDROID_LOGCAT)
    list(APPEND SUPPORT_LINK_LIBRARIES_PUBLIC ${ANDROID_LOG_LIB})
endif()

#
# breakpad
#
if(ENABLE_BREAKPAD)
    list(APPEND SUPPORT_DEFINITIONS_PUBLIC ENABLE_BREAKPAD)
endif(ENABLE_BREAKPAD)

#
# set client-specific sources, definitions, and compiler flags
#
list(APPEND SUPPORT_LINK_LIBRARIES_PUBLIC curl)
list(APPEND SUPPORT_LINK_LIBRARIES_PUBLIC nghttp2_static)

string(TOUPPER ${HTTP_CLIENT} HTTP_CLIENT_ID)
list(APPEND SUPPORT_DEFINITIONS_PUBLIC "HTTP_CLIENT_${HTTP_CLIENT_ID}")

# add useful definitions & remove non needed files
if (APPLE AND (NOT ANDROID))
    file(GLOB_RECURSE NETWORK_HTTP_SOURCES_OBJC "src/network/http/objc/*.mm")
    list(APPEND SUPPORT_SOURCES ${NETWORK_HTTP_SOURCES_OBJC})
    set_source_files_properties(${NETWORK_HTTP_SOURCES_OBJC} PROPERTY COMPILE_FLAGS "-Wno-deprecated-declarations")
endif(APPLE AND (NOT ANDROID))

if(HTTP_CLIENT STREQUAL "objc")
    if (NOT APPLE)
        message(FATAL_ERROR "HTTP_CLIENT cannot be set to 'objc', when the platform is not APPLE.")
    endif(NOT APPLE)
    list(APPEND SUPPORT_DEFINITIONS_PRIVATE "OBJC_HTTP_CLIENT")
endif()

# certificate map
if(NOT CERTIFICATE_MAP_FILE)
    set(CERTIFICATE_MAP_FILE support/network/DefaultCertificateMapping.h)
endif()
list(APPEND SUPPORT_DEFINITIONS_PRIVATE "CERTIFICATE_MAP_FILE=${CERTIFICATE_MAP_FILE}")

# enable JNI if that was explicitly required from outside
if(${ENABLE_JNI})
    list(APPEND SUPPORT_DEFINITIONS_PRIVATE "ENABLE_JNI")
    if (NOT ANDROID)
        find_package(JNI REQUIRED)
        list(APPEND SUPPORT_PRIVATE_INCLUDES ${JNI_INCLUDE_DIRS})
    endif()

    # add jni sources
    file(GLOB_RECURSE SUPPORT_SOURCES_JNI "src/jni/*.cpp" "include/support/jni/*.h")
    list(APPEND SUPPORT_SOURCES ${SUPPORT_SOURCES_JNI})
endif()

#
# set additional libraries
#

if (CMAKE_DL_LIBS)
    list(APPEND SUPPORT_LINK_LIBRARIES_PUBLIC ${CMAKE_DL_LIBS})
endif(CMAKE_DL_LIBS)

if(WIN32)
    list(APPEND SUPPORT_LINK_LIBRARIES_PUBLIC iphlpapi ws2_32)
endif(WIN32)

if(APPLE)
    find_library(FOUNDATION Foundation)
    find_library(CFNETWORK CFNetwork)
    find_library(SECURITY Security)
    list(APPEND SUPPORT_LINK_LIBRARIES_PUBLIC ${FOUNDATION} ${CFNETWORK} ${SECURITY})

    if(LOG_WITH_NSLOG)
        set_source_files_properties(logging/LoggerConsole.cpp PROPERTY COMPILE_FLAGS "-x objective-c++ -DLOG_WITH_NSLOG")
    endif(LOG_WITH_NSLOG)
endif()

find_package(Threads REQUIRED)
list(APPEND SUPPORT_LINK_LIBRARIES_PUBLIC Threads::Threads mbedcrypto)

#
# addistional definitions
#

if(HTTPS_PROXY_DISABLED)
    list(APPEND SUPPORT_DEFINITIONS_PUBLIC "HTTPS_PROXY_DISABLED=1")
endif(HTTPS_PROXY_DISABLED)

if(VERBOSE_HTTP_LOGGING)
    list(APPEND SUPPORT_DEFINITIONS_PUBLIC VERBOSE_HTTP_LOGGING)
endif(VERBOSE_HTTP_LOGGING)

# hue-arguments: mainly for unit/integration tests
if(HUE_ARGUMENTS)
    list(APPEND SUPPORT_DEFINITIONS_PRIVATE "HUE_ARGUMENTS=${HUE_ARGUMENTS}")
endif(HUE_ARGUMENTS)

#
# create support libraries
#

add_library(${PROJECT_NAME} STATIC ${SUPPORT_SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC ${SUPPORT_LINK_LIBRARIES_PUBLIC})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${SUPPORT_DEFINITIONS_PRIVATE})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${SUPPORT_DEFINITIONS_PUBLIC})
# add the root of this support repo to the include path
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(${PROJECT_NAME} PRIVATE ${SUPPORT_PRIVATE_INCLUDES})
set_target_properties(${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES)
treat_warning_as_error(${PROJECT_NAME})
add_dependencies(${PROJECT_NAME} boost)

add_subdirectory(mock)
