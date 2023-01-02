cmake_minimum_required(VERSION 3.6)

function(set_library_repo URL PREDEFINED_ARCHIVE_FILEPATH BASE_NAME VERSION LIBRARIES_INSTALL_PATH)
    if (NOT "${URL}" STREQUAL "")
        string(MD5 FLAGS_HASH "${CMAKE_CXX_FLAGS}${CMAKE_C_FLAGS}${CMAKE_CXX_FLAGS_DEBUG}${CMAKE_CXX_FLAGS_RELWITHDEBINFO}${CMAKE_SHARED_LINKER_FLAGS}")
        set(ARCHIVE_FILENAME "${BASE_NAME}_${VERSION}_${CMAKE_SYSTEM}_${FLAGS_HASH}.zip")

        if (NOT "${PREDEFINED_ARCHIVE_FILEPATH}" STREQUAL "")
            set(ARCHIVE_FILEPATH ${PREDEFINED_ARCHIVE_FILEPATH})
        elseif(NOT EXISTS ${PROJECT_BINARY_DIR}/${ARCHIVE_FILENAME})
            file(DOWNLOAD
                    "${URL}/${ARCHIVE_FILENAME}"
                    "${PROJECT_BINARY_DIR}/${ARCHIVE_FILENAME}"
                    STATUS DOWNLOAD_STATUS)
            list(GET DOWNLOAD_STATUS 0 DOWNLOAD_ERROR)
            if (NOT DOWNLOAD_ERROR)
                set(ARCHIVE_FILEPATH ${PROJECT_BINARY_DIR}/${ARCHIVE_FILENAME})
            endif()
        endif()

        if(EXISTS ${ARCHIVE_FILEPATH})
            file(REMOVE_RECURSE ${LIBRARIES_INSTALL_PATH})
            execute_process(
                    COMMAND ${CMAKE_COMMAND} -E tar xzf ${ARCHIVE_FILEPATH}
                    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
            )
        endif()

        # generate publish archive target just for local access to the repo (safty reason)
        if (${URL} MATCHES "file://.*")
            string(REPLACE "file://" "" URL ${URL})
            add_custom_target(publish_${BASE_NAME}_archive
                COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${URL}/${ARCHIVE_FILENAME}" --format=zip "${LIBRARIES_INSTALL_PATH}"
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
        endif()
    endif()
endfunction()