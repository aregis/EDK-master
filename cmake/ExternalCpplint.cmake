cmake_minimum_required(VERSION 3.6)

set(CPPLINT_DIR ${CMAKE_CURRENT_BINARY_DIR}/cpplint CACHE INTERNAL "")
message(STATUS "Downloading cpplint from ${cpplint_URL} into ${CPPLINT_DIR}")
set(EXTERNAL_CPPLINT "external_cpplint")
ExternalProject_Add(${EXTERNAL_CPPLINT}
    GIT_REPOSITORY ${cpplint_URL}
	GIT_TAG ${cpplint_VERSION}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )
set(CPPLINT_EXECUTABLE ${CPPLINT_DIR}/cpplint.py)
