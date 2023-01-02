cmake_minimum_required(VERSION 3.6)

include(ExternalProjectUtils)

set_external_library(json FALSE)

if(NOT EXISTS ${LIBRARY_PATH})
    set_library_path(json LIBJSON_PATH "")

    set(EXTERNAL_DEPENDENCY external_libjson)

    ExternalProject_Add(${EXTERNAL_DEPENDENCY}
            PREFIX ${EXTERNAL_LIBRARIES_BUILD_PATH}
DOWNLOAD_COMMAND ""
            UPDATE_COMMAND ""
            SOURCE_DIR "${EXTERNAL_LIBRARIES_SOURCE_PATH}/libjson"
            CMAKE_ARGS ${COMMON_ARGS} -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD} -DINSTALL_DESTINATION=include/libjson -DCMAKE_PREFIX_PATH=${EXTERNAL_LIBRARIES_INSTALL_PATH} -DCMAKE_INSTALL_PREFIX=${EXTERNAL_LIBRARIES_INSTALL_PATH}
            BUILD_BYPRODUCTS ${LIBJSON_PATH}
            LIST_SEPARATOR ^^
            )
endif(NOT EXISTS ${LIBRARY_PATH})

expose_external_library(STATIC)
