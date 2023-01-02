cmake_minimum_required(VERSION 3.6)

include(ExternalProjectUtils)

set_external_library(gtest FALSE)

if(NOT EXISTS ${LIBRARY_PATH})
    if(MINGW)
        set(PLATFORM_GOOGLETEST_OPTIONS -DCMAKE_USE_PTHREADS_INIT=0 -Dgtest_disable_pthreads=ON)
    endif(MINGW)

    if (MSVC)
        set(GTEST_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
    else(MSVC)
        set(GTEST_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    endif(MSVC)

    set_library_path(gtest GTEST_PATH "")
    set_library_path(gmock GMOCK_PATH "")

    set(EXTERNAL_DEPENDENCY external_googletest)

    ExternalProject_Add(${EXTERNAL_DEPENDENCY}
        PREFIX ${EXTERNAL_LIBRARIES_BUILD_PATH}
        DOWNLOAD_COMMAND ${CMAKE_COMMAND} -DREFERENCE=${GIT_REFERENCE} -DREPO=${googletest_URL} -DBRANCH=${googletest_VERSION} -DSOURCE_DIR=${EXTERNAL_LIBRARIES_SOURCE_PATH}/googletest -P ${CMAKE_CURRENT_LIST_DIR}/CloneRepository.cmake
        UPDATE_COMMAND ""
        SOURCE_DIR "${EXTERNAL_LIBRARIES_SOURCE_PATH}/googletest"
        CMAKE_CACHE_ARGS -DBUILD_DTLS_LIB:BOOL=OFF -DBUILD_TEST:BOOL=OFF -DBUILD_CLIENT:BOOL=ON -DBUILD_SERVER:BOOL=OFF
        CMAKE_ARGS ${COMMON_ARGS} -Dgtest_force_shared_crt=ON ${PLATFORM_GOOGLETEST_OPTIONS} -DCMAKE_C_FLAGS=${DCMAKE_C_FLAGS} -DCMAKE_CXX_FLAGS=${GTEST_CXX_FLAGS}
        BUILD_BYPRODUCTS ${GTEST_PATH} ${GMOCK_PATH}
        LIST_SEPARATOR ^^
    )
endif(NOT EXISTS ${LIBRARY_PATH})

expose_external_library(STATIC)
expose_additional_external_library(gmock)
expose_additional_external_library(gtest_main)
