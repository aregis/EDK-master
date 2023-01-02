add_library(edtls_client STATIC DTLSClient.cpp DTLSClient.h IDTLSClient.h IClientNotifier.h ../server/ISessionHandler.h ../wrapper/IClientConnectionHandler.h ../wrapper/IClientWrapper.h ../logger/Logger.h)
target_link_libraries(edtls_client mbedcl_wrapper)

install(TARGETS edtls_client DESTINATION lib)
install(DIRECTORY ../../edtls/client
        DESTINATION include/edtls
        FILES_MATCHING PATTERN "*.h")
