cmake_minimum_required(VERSION 3.6)

# set external project paths
set(EXTERNAL_LIBRARIES_BUILD_PATH ${PROJECT_BINARY_DIR}/external_build)
set(EXTERNAL_LIBRARIES_INSTALL_PATH ${PROJECT_BINARY_DIR}/external_install)
set(EXTERNAL_LIBRARIES_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# create empty instalation folder in order to avoid cmake's includes non-existen path warnings
file(MAKE_DIRECTORY ${EXTERNAL_LIBRARIES_INSTALL_PATH}/include)

# download and compile external projects
if (BUILD_GTEST)
    include(ExternalGoogleTest)
endif()

if (BUILD_DTLS_LIB)
    include(ExternalMbedtls)
endif()
