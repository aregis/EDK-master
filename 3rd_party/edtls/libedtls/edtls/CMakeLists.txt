add_subdirectory(wrapper/mbedtls)

if (MSVC)
    set(CMAKE_TARGET_SPECIFIC_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
else()
    set(CMAKE_TARGET_SPECIFIC_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++ -Werror")
endif()

install(DIRECTORY ../edtls/logger
        DESTINATION include/edtls
        FILES_MATCHING PATTERN "*.h")

install(FILES
        ../edtls/wrapper/helpers.h
        ../edtls/wrapper/PSKInfo.h
        DESTINATION include/edtls/wrapper)

set(depends "")

if (BUILD_CLIENT)
    add_subdirectory(client)
    set_target_properties(mbedcl_wrapper PROPERTIES COMPILE_FLAGS ${CMAKE_TARGET_SPECIFIC_CXX_FLAGS})
    set_target_properties(edtls_client PROPERTIES COMPILE_FLAGS ${CMAKE_TARGET_SPECIFIC_CXX_FLAGS})
    install(FILES
            ../edtls/wrapper/IClientConnectionHandler.h
            ../edtls/wrapper/IClientWrapper.h
            DESTINATION include/edtls/wrapper)
    set(depends ${depends} edtls_client)
endif ()

if (BUILD_SERVER)
    add_subdirectory(server)
    set_target_properties(edtls_mbedtls_server_wrapper PROPERTIES COMPILE_FLAGS ${CMAKE_TARGET_SPECIFIC_CXX_FLAGS})
    set_target_properties(edtls_server PROPERTIES COMPILE_FLAGS ${CMAKE_TARGET_SPECIFIC_CXX_FLAGS})
    install(FILES
            ../edtls/wrapper/IPSKProvider.h
            ../edtls/wrapper/IServerWrapper.h
            ../edtls/wrapper/ISessionHandler.h
            ../edtls/wrapper/PSKInfo.h
            DESTINATION include/edtls/wrapper)
    set(depends ${depends} edtls_server)
endif ()

add_custom_target(edtls_install
        ${MAKE} install
        DEPENDS ${depends}
        COMMENT "Installing ... ")