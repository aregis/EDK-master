cmake_minimum_required(VERSION 3.6)

project(bridge_discovery_tests)

file(GLOB_RECURSE SOURCES "*.h" "*.cpp")

add_test_library(${PROJECT_NAME} "${SOURCES}")
target_include_directories(${PROJECT_NAME} PUBLIC "./include/" "./src/")
target_link_libraries(${PROJECT_NAME} PRIVATE gmock gtest huesdk_core testrunner bridge_discovery_mock)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED YES)
