cmake_minimum_required(VERSION 3.6)

include(ExternalProject)

# CMake unfortunately does *NOT* forward all variables when configuring an
# external project, most notably cross-compilation variables (TOOLCHAIN etc)
# which are used for Android and iOS builds.

if(ANDROID)
    set(COMMON_ARGS -DANDROID_STL=c++_static -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM} -DANDROID_NDK=${ANDROID_NDK} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL} -DANDROID_ABI=${ANDROID_ABI})
endif()

if(APPLE)
    # We cannot pass all CMAKE_C_FLAGS arguments (lto, embed bitcode, etc)
    # through CMAKE_ARGS because cmake inserts quotes in the wrong places.
    # Therefore we pass custom cmake arguments and handle them in the patched
    # CMakeLists.txt file inside the external project.

    # We also need to fix up CMAKE_OSX_ARCHITECTURES on the other side because
    # it gets truncated at the semicolon (e.g. "arm64;armv7").

    set(COMMON_ARGS -DAPPLE_PLATFORM=${APPLE_PLATFORM} -DENABLE_LTO=${ENABLE_LTO} -DENABLE_EMBED_BITCODE=${ENABLE_EMBED_BITCODE} -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES} -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT} -DCMAKE_C_FLAGS=${DEPLOYMENT_TARGET} -DCMAKE_CXX_FLAGS=${DEPLOYMENT_TARGET})
endif()

# All external libraries are compiled in RELEASE mode

set(COMMON_ARGS ${COMMON_ARGS} -DCMAKE_INSTALL_PREFIX=${EXTERNAL_LIBRARIES_INSTALL_PATH} -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER})


# Create a Cmake library
#
function(set_library_path LIBRARY_NAME LIBRARY_PATH_VAR LIBRARY_FILE_PATH)
    if ("${LIBRARY_FILE_PATH}" STREQUAL "")
        set(LIBRARY_FILE_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}${LIBRARY_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})
    else()
        set(LIBRARY_FILE_NAME ${LIBRARY_FILE_PATH})
    endif()
    set(${LIBRARY_PATH_VAR} "${EXTERNAL_LIBRARIES_INSTALL_PATH}/lib/${LIBRARY_FILE_NAME}" PARENT_SCOPE)
endfunction()

# Setup imported libraries with the transitive cmake properties, so that you
# can simply target_link_libraries(mylib curl) and have it automatically
# receive the compile definitions, include paths, and all link dependencies.

function(expose_external_library LIBRARY_NAME EXTERNAL_DEPENDENCY LINK_DEPENDENCY LIBRARY_FILE_PATH)
    add_library(${LIBRARY_NAME} STATIC IMPORTED GLOBAL)
    add_dependencies(${LIBRARY_NAME} ${EXTERNAL_DEPENDENCY})
    set_library_path(${LIBRARY_NAME} LIBRARY_PATH "${LIBRARY_FILE_PATH}")
    set_property(TARGET ${LIBRARY_NAME} PROPERTY IMPORTED_LOCATION ${LIBRARY_PATH})
    set_property(TARGET ${LIBRARY_NAME} APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${EXTERNAL_LIBRARIES_INSTALL_PATH}/include)
    set_property(TARGET ${LIBRARY_NAME} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${LINK_DEPENDENCY})
endfunction()
