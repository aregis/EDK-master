function(apply_and_check_patch patch)
    set(git_cmd "git")
    set(git_arg "apply;--whitespace=fix;--check;${patch}")
    execute_process(COMMAND ${git_cmd} ${git_arg}
            RESULT_VARIABLE FAILED
            OUTPUT_QUIET
            ERROR_QUIET)

    if (NOT FAILED)
        message("Patch ${patch} can be applied :-)")
        set(git_cmd "git")
        set(git_arg "apply;--whitespace=fix;${patch}")
        execute_process(COMMAND ${git_cmd} ${git_arg}
                RESULT_VARIABLE FAILED
                OUTPUT_QUIET
                ERROR_QUIET)
        if (NOT FAILED)
            message("Patch ${patch} is applied :-)")
        else ()
            message(FATAL_ERROR "failed applying patch ${patch}:-(")
        endif ()
    else ()
        message(WARNING "Patch ${patch} cannot applied :-(")
    endif ()
endfunction()

apply_and_check_patch("${PATCH}")