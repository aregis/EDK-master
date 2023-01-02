cmake_minimum_required(VERSION 3.6)

include(ExternalProjectUtils)

if(MINGW)
    set(PLATFORM_GOOGLETEST_OPTIONS -DCMAKE_USE_PTHREADS_INIT=0 -Dgtest_disable_pthreads=ON)
endif(MINGW)

set_library_path(gtest GTEST_PATH "")
set_library_path(gmock GMOCK_PATH "")

ExternalProject_Add(external_googletest
    PREFIX ${EXTERNAL_LIBRARIES_BUILD_PATH}
    DOWNLOAD_COMMAND ${CMAKE_COMMAND} -DREFERENCE=${GIT_REFERENCE} -DREPO=https://github.com/google/googletest.git -DBRANCH=release-1.8.0 -DSOURCE_DIR=${EXTERNAL_LIBRARIES_SOURCE_PATH}/googletest -P ${CMAKE_CURRENT_LIST_DIR}/CloneRepository.cmake
    UPDATE_COMMAND ""
    SOURCE_DIR "${EXTERNAL_LIBRARIES_SOURCE_PATH}/googletest"
    CMAKE_CACHE_ARGS -DBUILD_DTLS_LIB:BOOL=OFF -DBUILD_TEST:BOOL=OFF -DBUILD_CLIENT:BOOL=ON -DBUILD_SERVER:BOOL=OFF
    CMAKE_ARGS ${COMMON_ARGS} -Dgtest_force_shared_crt=ON ${PLATFORM_GOOGLETEST_OPTIONS} -DCMAKE_C_FLAGS=${DCMAKE_C_FLAGS} -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    BUILD_BYPRODUCTS ${GTEST_PATH} ${GMOCK_PATH}
)

expose_external_library(gtest external_googletest "" "")
expose_external_library(gmock external_googletest "" "")
expose_external_library(gtest_main external_googletest "" "")
