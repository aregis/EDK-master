cmake_minimum_required(VERSION 3.6)

include(ExternalProjectUtils)

set_external_library(mbedcrypto FALSE)

if(NOT EXISTS ${LIBRARY_PATH})
    set_library_path(mbedcrypto MBEDCRYPTO_PATH "")
    set_library_path(mbedx509   MBEDX509_PATH "")
    set_library_path(mbedtls    MBEDTLS_PATH "")    

    set(EXTERNAL_DEPENDENCY external_mbedtls)

    ExternalProject_Add(${EXTERNAL_DEPENDENCY}
        PREFIX ${EXTERNAL_LIBRARIES_BUILD_PATH}
        DOWNLOAD_COMMAND ${CMAKE_COMMAND} -DREFERENCE=${GIT_REFERENCE} -DREPO=${mbedtls_URL} -DBRANCH=${mbedtls_VERSION} -DSOURCE_DIR=${EXTERNAL_LIBRARIES_SOURCE_PATH}/mbedtls -DPATCH=mbedtls.patch -P ${CMAKE_CURRENT_LIST_DIR}/CloneRepository.cmake
        UPDATE_COMMAND ""
        SOURCE_DIR "${EXTERNAL_LIBRARIES_SOURCE_PATH}/mbedtls"
        CMAKE_ARGS ${COMMON_ARGS} -DUSE_STATIC_MBEDTLS_LIBRARY=ON -DUSE_SHARED_MBEDTLS_LIBRARY=OFF -DENABLE_TESTING=OFF -DENABLE_PROGRAMS=OFF -DMBEDTLS_FATAL_WARNINGS=OFF
        BUILD_BYPRODUCTS ${MBEDCRYPTO_PATH} ${MBEDX509_PATH} ${MBEDTLS_PATH}
        LIST_SEPARATOR ^^
    )
endif(NOT EXISTS ${LIBRARY_PATH})

expose_external_library(STATIC)
expose_additional_external_library(mbedx509 mbedcrypto)
expose_additional_external_library(mbedtls mbedx509)

if(WIN32)
    set_property(TARGET mbedtls APPEND PROPERTY INTERFACE_LINK_LIBRARIES iphlpapi ws2_32)
endif(WIN32)