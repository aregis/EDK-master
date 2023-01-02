set(LIBS ${CMAKE_THREAD_LIBS_INIT})

if (NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
endif ()

include_directories(
        ${CMAKE_PREFIX_PATH}/include
        .
)

link_directories(
        ${CMAKE_PREFIX_PATH}/lib
)

add_subdirectory(edtls)

if (BUILD_TEST)
    add_subdirectory(test)
endif ()