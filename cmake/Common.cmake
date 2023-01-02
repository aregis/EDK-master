cmake_minimum_required(VERSION 3.6)

function(get_current_target_property TARGET PROPERTY RESULT_NAME)
    get_target_property(CURRENT_PROPERTIES ${TARGET} ${PROPERTY})
    if(${CURRENT_PROPERTIES} STREQUAL "CURRENT_PROPERTIES-NOTFOUND")
        set(CURRENT_PROPERTIES "")
    endif()
    set(${RESULT_NAME} ${CURRENT_PROPERTIES} PARENT_SCOPE)
endfunction()

function(treat_warning_as_error TARGET)
    get_current_target_property(${TARGET} COMPILE_FLAGS CURRENT_COMPILE_FLAGS)
    if(ENABLE_WERROR)
        if(MSVC)
            set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS "${CURRENT_COMPILE_FLAGS} /W3 /wd4373 /wd4068 /wd4250 /wd4351 /wd4996 /WX")
        else(MSVC)
            set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS "${CURRENT_COMPILE_FLAGS} -Wall -Wextra -Werror")
        endif(MSVC)
    endif(ENABLE_WERROR)
endfunction()

function(linker_no_deduplicate TARGET)
    get_current_target_property(${TARGET} LINK_FLAGS CURRENT_LINK_FLAGS)
    if(APPLE)
        set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "${CURRENT_LINK_FLAGS} -Xlinker -no_deduplicate")
    endif(APPLE)
endfunction()


function(add_test_library PROJECT_NAME SOURCES)
    if(MSVC)
        set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS /bigobj)
    elseif(WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "-Wno-inconsistent-missing-override -Os")
    else(MSVC)
        set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS -Wno-inconsistent-missing-override)
    endif(MSVC)

    if(ANDROID)
        add_library(${PROJECT_NAME} SHARED ${SOURCES} "$<TARGET_PROPERTY:testrunner,SOURCE_DIR>/JNITestRunner.cpp")
    else(OTHER)
        add_executable(${PROJECT_NAME} ${SOURCES})
    endif()

    linker_no_deduplicate(${PROJECT_NAME})
    treat_warning_as_error(${PROJECT_NAME})
endfunction()