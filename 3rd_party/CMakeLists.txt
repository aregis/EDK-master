cmake_minimum_required(VERSION 3.6)

# set external project paths
set(EXTERNAL_LIBRARIES_BUILD_PATH ${PROJECT_BINARY_DIR}/external_build)
set(EXTERNAL_LIBRARIES_INSTALL_PATH ${PROJECT_BINARY_DIR}/external_install)
set(EXTERNAL_LIBRARIES_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# create empty instalation folder in order to avoid cmake's includes non-existing path warnings
file(MAKE_DIRECTORY ${EXTERNAL_LIBRARIES_INSTALL_PATH}/include)

# download and compile external projects
include(ExternalJson)
include(ExternalGoogleTest)
include(ExternalMbedtls)
include(ExternalEdtls)
include(ExternalMDNSResponder)
include(ExternalNghttp2)
include(ExternalCurl)
include(ExternalBoost)
