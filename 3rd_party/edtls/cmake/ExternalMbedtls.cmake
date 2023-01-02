cmake_minimum_required(VERSION 3.6)

include(ExternalProjectUtils)

set_library_path(mbedcrypto MBEDCRYPTO_PATH "")
set_library_path(mbedx509   MBEDX509_PATH "")
set_library_path(mbedtls    MBEDTLS_PATH "")

# The patch for mbedTLS defines the mbedtls configuration header file. I
# tried including mbedtls-config.h through CMAKE_ARGS but that did not
# work because CMake tried to use it during the identification of the
# cross-compilers.

ExternalProject_Add(external_mbedtls
    PREFIX ${EXTERNAL_LIBRARIES_BUILD_PATH}
    DOWNLOAD_COMMAND ${CMAKE_COMMAND} -DREFERENCE=${GIT_REFERENCE} -DREPO=https://github.com/ARMmbed/mbedtls.git -DBRANCH=mbedtls-2.3.0 -DSOURCE_DIR=${EXTERNAL_LIBRARIES_SOURCE_PATH}/mbedtls -P ${CMAKE_CURRENT_LIST_DIR}/CloneRepository.cmake
    UPDATE_COMMAND ""
    SOURCE_DIR "${EXTERNAL_LIBRARIES_SOURCE_PATH}/mbedtls"
    CMAKE_ARGS ${COMMON_ARGS} -DUSE_STATIC_MBEDTLS_LIBRARY=ON -DUSE_SHARED_MBEDTLS_LIBRARY=OFF -DENABLE_TESTING=OFF -DENABLE_PROGRAMS=OFF
    BUILD_BYPRODUCTS ${MBEDCRYPTO_PATH} ${MBEDX509_PATH} ${MBEDTLS_PATH}
)

expose_external_library(mbedcrypto external_mbedtls "" "")
expose_external_library(mbedx509   external_mbedtls mbedcrypto "")
expose_external_library(mbedtls    external_mbedtls mbedx509 "")
