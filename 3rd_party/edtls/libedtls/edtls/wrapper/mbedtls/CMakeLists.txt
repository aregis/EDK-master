set(LIBS ${CMAKE_THREAD_LIBS_INIT} mbedtls mbedcrypto)

if (WIN32)
    set(LIBS ${LIBS} -lws2_32)
endif ()

if (BUILD_CLIENT)
    add_library(mbedcl_wrapper STATIC
            MbedtlsClientWrapper.cpp
            MbedtlsClientWrapper.h
            MbedtlsClientWrapperFactory.cpp
            MbedtlsClientWrapperFactory.h
            UDPBase.h
            UDPClientBase.h
            ../helpers.h
            EntropyProviderBase.h
            DefaultEntropyProvider.cpp
            DefaultEntropyProvider.h
            DefaultTimerProvider.cpp
            DefaultTimerProvider.h
            DefaultUDPClient.cpp
            DefaultUDPClient.h
            MbedtlsPlatform.cpp
            MbedtlsPlatform.h
            MbedtlsClientPlatform.cpp
            MbedtlsClientPlatform.h TimerProvider.h)
    target_link_libraries(mbedcl_wrapper ${LIBS})
    install(TARGETS mbedcl_wrapper DESTINATION lib)
    install(FILES
            ../../../edtls/wrapper/mbedtls/MbedtlsClientWrapper.h
            ../../../edtls/wrapper/mbedtls/MbedtlsClientWrapperFactory.h
            ../../../edtls/wrapper/mbedtls/UDPBase.h
            ../../../edtls/wrapper/mbedtls/UDPClientBase.h
            ../../../edtls/wrapper/mbedtls/EntropyProviderBase.h
            ../../../edtls/wrapper/mbedtls/DefaultEntropyProvider.h
            ../../../edtls/wrapper/mbedtls/DefaultTimerProvider.h
            ../../../edtls/wrapper/mbedtls/DefaultUDPClient.h
            ../../../edtls/wrapper/mbedtls/TimerProvider.h
            ../../../edtls/wrapper/mbedtls/MbedtlsPlatform.h
            ../../../edtls/wrapper/mbedtls/MbedtlsClientPlatform.h
            DESTINATION include/edtls/wrapper/mbedtls)
endif ()

if (BUILD_SERVER)
    add_library(edtls_mbedtls_server_wrapper STATIC
            MbedtlsServerWrapper.cpp
            MbedtlsServerWrapper.h
            MbedtlsServerWrapperFactory.cpp
            MbedtlsServerWrapperFactory.h
            UDPBase.h
            UDPServerBase.h
            ../helpers.h
            ../IPSKProvider.h
            MbedtlsPlatform.cpp
            MbedtlsPlatform.h
            MbedtlsServerPlatform.cpp
            MbedtlsServerPlatform.h
            TimerProvider.h)
    target_link_libraries(edtls_mbedtls_server_wrapper ${LIBS})
    install(TARGETS edtls_mbedtls_server_wrapper DESTINATION lib)

    install(FILES
            ../../../edtls/wrapper/mbedtls/EntropyProviderBase.h
            ../../../edtls/wrapper/mbedtls/MbedtlsServerWrapper.h
            ../../../edtls/wrapper/mbedtls/MbedtlsServerWrapperFactory.h
            ../../../edtls/wrapper/mbedtls/MbedtlsServerPlatform.h
            ../../../edtls/wrapper/mbedtls/MbedtlsPlatform.h
            ../../../edtls/wrapper/mbedtls/UDPBase.h
            ../../../edtls/wrapper/mbedtls/UDPServerBase.h
            ../../../edtls/wrapper/mbedtls/TimerProvider.h
            DESTINATION include/edtls/wrapper/mbedtls)
endif ()