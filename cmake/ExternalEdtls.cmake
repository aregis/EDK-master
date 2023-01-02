cmake_minimum_required(VERSION 3.6)

include(ExternalProjectUtils)

set_external_library(edtls_client FALSE mbedtls)

if(NOT EXISTS ${LIBRARY_PATH})
    set_library_path(edtls_client   EDTLS_CLIENT_PATH "")
    set_library_path(mbedcl_wrapper CLIENT_WRAPPER_PATH "")

    option(BUILD_WITHOUT_RTTI "Build without RTTI support" ON)

    set(EXTERNAL_DEPENDENCY external_edtls)

    ExternalProject_Add(${EXTERNAL_DEPENDENCY}
            PREFIX ${EXTERNAL_LIBRARIES_BUILD_PATH}
DOWNLOAD_COMMAND ""
            UPDATE_COMMAND ""
            SOURCE_DIR "${EXTERNAL_LIBRARIES_SOURCE_PATH}/edtls"
            CMAKE_CACHE_ARGS -DBUILD_DTLS_LIB:BOOL=OFF -DBUILD_WITHOUT_RTTI:BOOL=${BUILD_WITHOUT_RTTI} -DBUILD_TEST:BOOL=OFF -DBUILD_CLIENT:BOOL=ON -DBUILD_SERVER:BOOL=OFF
            CMAKE_ARGS ${COMMON_ARGS} -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD} -DCMAKE_PREFIX_PATH=${EXTERNAL_LIBRARIES_INSTALL_PATH} -DCMAKE_INSTALL_PREFIX=${EXTERNAL_LIBRARIES_INSTALL_PATH}
            DEPENDS mbedtls
            BUILD_BYPRODUCTS ${EDTLS_CLIENT_PATH} ${CLIENT_WRAPPER_PATH}
            LIST_SEPARATOR ^^
            )
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_BINARY_DIR}/edtls")
endif(NOT EXISTS ${LIBRARY_PATH})

expose_external_library(STATIC)
expose_additional_external_library(mbedcl_wrapper)
