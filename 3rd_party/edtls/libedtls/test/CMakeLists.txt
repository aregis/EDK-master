
set(LIBS edtls_client edtls_server)
set(LIBS ${LIBS} gtest gmock gtest_main)

add_executable(edtls_tests
        TestDTLS.cpp
        )

target_link_libraries(edtls_tests ${LIBS})

add_custom_command(TARGET edtls_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_PREFIX_PATH}/lib"
        $<TARGET_FILE_DIR:edtls_tests>)