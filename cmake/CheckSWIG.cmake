# Get SWIG major version
function(check_swig major minor build)
    execute_process(
            COMMAND ${SWIG_EXECUTABLE} -version
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE
            res
            OUTPUT_VARIABLE
            output
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REGEX REPLACE "SWIG Version ([0-9]+)\\.([0-9]+)\\.([0-9]+).*" "\\1" smajor sminor sbuild ${output})
    string(STRIP ${swig_major_ver} smajor)
    string(STRIP ${swig_major_ver} sminor)
    string(STRIP ${swig_major_ver} sbuild)
    set(${major} ${smajor} PARENT_SCOPE)
    set(${minor} ${smajor} PARENT_SCOPE)
    set(${build} ${smajor} PARENT_SCOPE)
endfunction()