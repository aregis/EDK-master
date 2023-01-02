function(git_cmd args dir OUTPUT_VARIABLE RESULT_VARIABLE)
    execute_process(COMMAND git ${args}
            WORKING_DIRECTORY ${dir}
            OUTPUT_VARIABLE OUTPUT
            RESULT_VARIABLE RESULT
            ERROR_QUIET
            )
    set(${OUTPUT_VARIABLE} "${OUTPUT}" PARENT_SCOPE)
    set(${RESULT_VARIABLE} ${RESULT} PARENT_SCOPE)
endfunction()

function(git_has_local_changes SOURCE_DIR OUT_HAS_LOCAL_CHANGES)
    git_cmd("status;--porcelain" ${SOURCE_DIR} OUTPUT RESULT)
    if (OUTPUT)
        set(${OUT_HAS_LOCAL_CHANGES} TRUE PARENT_SCOPE)
    else ()
        set(${OUT_HAS_LOCAL_CHANGES} FALSE PARENT_SCOPE)
    endif ()
endfunction()

function(git_get_current_revision SOURCE_DIR OUT_REVISION)
    git_cmd("rev-parse;HEAD" ${SOURCE_DIR} OUTPUT RESULT)
    string(STRIP "${OUTPUT}" OUTPUT)
    set(${OUT_REVISION} ${OUTPUT} PARENT_SCOPE)
endfunction()

function(git_get_url SOURCE_DIR OUT_URL)
    git_cmd("config;--get;remote.origin.url" ${SOURCE_DIR} OUTPUT RESULT)
    string(STRIP "${OUTPUT}" OUTPUT)
    set(${OUT_URL} ${OUTPUT} PARENT_SCOPE)
endfunction()

function(git_get_archive_name SOURCE_DIR OUT_NAME)
    git_get_url(${SOURCE_DIR} GIT_URL)
    string(REGEX MATCH "[^/]*.git$" GIT_ARCHIVE_NAME "${GIT_URL}")
    set(${OUT_NAME} ${GIT_ARCHIVE_NAME} PARENT_SCOPE)
endfunction()

function(generate_build_info SOURCE_DIR)
    git_has_local_changes(${SOURCE_DIR} LOCAL_CHANGES)
    git_get_current_revision(${SOURCE_DIR} GIT_REVISION)
    git_get_archive_name(${SOURCE_DIR} GIT_ARCHIVE_NAME)

    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "UNKNOWN_BUILD_TYPE")
    endif ()

    string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)

    set(BUILD_INFO "${CMAKE_BUILD_TYPE}@${GIT_ARCHIVE_NAME}:${GIT_REVISION}")

    if (${LOCAL_CHANGES})
        if (${CMAKE_BUILD_TYPE} MATCHES "RELEASE")
            message(WARNING "Build contains local changes. Commit them to get a build revision.")
        endif ()
        set(BUILD_INFO "${BUILD_INFO}-dirty")
    endif ()

    message("BUILD_INFO code define contains buildstring ${BUILD_INFO}")
    add_definitions(-DBUILD_INFO="${BUILD_INFO}")
endfunction()