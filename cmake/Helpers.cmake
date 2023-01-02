macro (copy_libs_to_output target libs )
    foreach(lib ${libs})
  #      add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${lib}>" $<TARGET_FILE_DIR:${target}>)
    endforeach()
endmacro()

macro(copy_to_binary_dir target file)
    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E
            copy ${file} ${BUILD_DIR})
    install(FILES ${file} DESTINATION ${INSTALL_DIR})
endmacro()

macro(copy_target_to_binary_dir target)
    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E
            copy_if_different "$<TARGET_FILE:${target}>" ${BUILD_DIR})
endmacro()


macro(add_shared_library target libs)
    set_target_properties(${target} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    if(WIN32)
        set_target_properties(${target} PROPERTIES IMPORT_SUFFIX "_imp.lib")
        set_target_properties(${target} PROPERTIES PREFIX "")
    endif()

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
        generate_export_header(${target})
    endif()
    target_link_libraries(${target} ${libs})
    set(shared_libs ${shared_libs} ${target})
    set(shared_libs ${shared_libs} PARENT_SCOPE)
endmacro()

macro(add_static_library target libs)
    if(WIN32)
        set_target_properties(${target} PROPERTIES SUFFIX ".lib")
        set_target_properties(${target} PROPERTIES PREFIX "")

    endif()
    set(shared_libs ${libs} ${shared_libs})
    set(shared_libs ${target} ${shared_libs})
    set(shared_libs ${shared_libs} PARENT_SCOPE)
    target_link_libraries(${target} ${libs})
    copy_target_to_binary_dir(${target})
endmacro()

macro(add_library_dependencies target libs)
    if (BUILD_SHARED)
        add_shared_library(${target} "${LIBS}")
    endif()

    if (BUILD_STATIC)
        add_static_library(${target} "${LIBS}")
    endif()
endmacro()

