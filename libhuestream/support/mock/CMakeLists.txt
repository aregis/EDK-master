cmake_minimum_required(VERSION 3.6)

project(support_mock)

list(APPEND SOURCES
    include/support/mock/network/MockNetworkDelegate.h
    include/support/mock/network/http/MockHttpRequest.h
    include/support/mock/network/http/MockHttpRequestDelegateProvider.h
    include/support/mock/network/http/MockHttpResponse.h
    include/support/mock/network/sockets/MockSocketUdp.h
    include/support/mock/network/sockets/MockSocketUdpDelegateProvider.h
    include/support/mock/network/sockets/bsd/MockCMethodDelegate.h
    src/network/http/MockHttpRequest.cpp
    src/network/sockets/MockSocketUdp.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
treat_warning_as_error(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC support gmock gtest)
