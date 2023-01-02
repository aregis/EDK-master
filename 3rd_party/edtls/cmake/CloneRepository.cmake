cmake_minimum_required(VERSION 3.6)

if(REFERENCE)
    set(CLONE_ARGS --reference ${REFERENCE})
else()
    set(CLONE_ARGS)
endif()

if(NOT EXISTS ${SOURCE_DIR}/CMakeLists.txt)
    execute_process(COMMAND git clone ${CLONE_ARGS} --branch ${BRANCH} ${REPO} ${SOURCE_DIR})

    if(PATCH)
        execute_process(COMMAND git apply --ignore-space-change --ignore-whitespace ${CMAKE_CURRENT_LIST_DIR}/patches/${PATCH}
            WORKING_DIRECTORY ${SOURCE_DIR})
    endif()
endif()
