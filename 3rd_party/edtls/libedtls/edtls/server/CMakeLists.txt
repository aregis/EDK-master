add_library(edtls_server STATIC DTLSServer.cpp DTLSServer.h IDTLSServer.h IServerNotifier.h ../wrapper/IServerWrapper.h MessageDispatcher.cpp MessageDispatcher.h Queue.h Semaphore.h ISessionHandler.h ../logger/Logger.h)
target_link_libraries(edtls_server edtls_mbedtls_server_wrapper)

install(TARGETS edtls_server DESTINATION lib)
install(DIRECTORY ../../edtls/server
        DESTINATION include/edtls
        FILES_MATCHING PATTERN "*.h")
