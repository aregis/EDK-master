cmake_minimum_required(VERSION 3.6)

project(bridge_discovery_mock)

file(GLOB_RECURSE SOURCES include/*.h src/*.h src/*.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ./include/ ../src/)
target_link_libraries(${PROJECT_NAME} PUBLIC bridge_discovery support_mock)
treat_warning_as_error(${PROJECT_NAME})
