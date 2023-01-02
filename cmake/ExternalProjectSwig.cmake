# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
    return()
endif()

set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED Swig_DIR AND NOT EXISTS ${Swig_DIR})
    message(FATAL_ERROR "Swig_DIR variable is defined but corresponds to non-existing directory")
endif()

if(WIN32 OR ${CMAKE_SYSTEM_NAME} STREQUAL "Android")	
	set(SWIGWIN_URL "https://downloads.sourceforge.net/project/swig/swigwin/swigwin-${SWIG_VERSION}/swigwin-${SWIG_VERSION}.zip?r=https%3A%2F%2Fsourceforge.net%2Fprojects%2Fswig%2Ffiles%2Fswigwin%2Fswigwin-${SWIG_VERSION}%2Fswigwin-${SWIG_VERSION}.zip%2Fdownload%3Fuse_mirror%3Dnetix&ts=1538981168")
    # binary SWIG for windows
    #------------------------------------------------------------------------------
    set(swig_source_dir ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_VERSION})

    # swig.exe available as pre-built binary on Windows:
	if(DEFINED ENV{CI})
		ExternalProject_Add(swig_build				
				DOWNLOAD_COMMAND curl -L ${SWIGWIN_URL} --output ${CMAKE_CURRENT_BINARY_DIR}/swig_build-prefix/src/swigwin-${SWIG_VERSION}.zip
				COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR} tar xzf ${CMAKE_CURRENT_BINARY_DIR}/swig_build-prefix/src/swigwin-${SWIG_VERSION}.zip
				SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_VERSION}
				CONFIGURE_COMMAND ""
				BUILD_COMMAND ""
				INSTALL_COMMAND ""
				)
	else()
		ExternalProject_Add(swig_build
				URL "${SWIGWIN_URL}"
				URL_MD5 ${SWIG_DOWNLOAD_WIN_HASH}				
				SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_VERSION}
				CONFIGURE_COMMAND ""
				BUILD_COMMAND ""
				INSTALL_COMMAND ""
				)
	endif()

    set(SWIG_DIR   ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_VERSION}/Lib CACHE INTERNAL "SWIG_DIR")
    set(SWIG_EXECUTABLE   ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_VERSION}/swig.exe CACHE INTERNAL "SWIG_EXECUTABLE")
else()
    #todo check dependencies

    set(SWIG_BUILD_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/swig_build-prefix/src/swig_build CACHE INTERNAL "")
    set(SWIG_BUILD_BIN_DIR    ${CMAKE_CURRENT_BINARY_DIR}/swig CACHE INTERNAL "")
    message(STATUS "Downloading sources from ${SWIG_URL}")

    if (${SWIG_URL} MATCHES ".+(\.git)$")
        ExternalProject_Add(swig_build
                GIT_REPOSITORY ${SWIG_URL}
                GIT_TAG ${SWIG_VERSION}
                UPDATE_COMMAND ""
                CONFIGURE_COMMAND ${SWIG_BUILD_SOURCE_DIR}/configure --without-clisp --without-maximum-compile-warnings --prefix=${SWIG_BUILD_BIN_DIR}
                )

    else()
        set(SWIG_BUILD_HASH 82133dfa7bba75ff9ad98a7046be687c)
        set(SWIG_BUILD_URL ${SWIG_URL})
        ExternalProject_Add(swig_build
                URL "${SWIG_BUILD_URL}"
                URL_MD5 ${SWIG_BUILD_HASH}
                UPDATE_COMMAND ""
                CONFIGURE_COMMAND ${SWIG_BUILD_SOURCE_DIR}/configure --without-clisp --without-maximum-compile-warnings --prefix=${SWIG_BUILD_BIN_DIR}
                )
    endif()

    set(SWIG_DIR   ${SWIG_BUILD_BIN_DIR} CACHE INTERNAL "SWIG_DIR")
    set(SWIG_EXECUTABLE  ${SWIG_BUILD_BIN_DIR}/bin/swig CACHE INTERNAL "SWIG_EXECUTABLE")
endif()
