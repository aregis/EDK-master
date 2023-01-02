if (WIN32 AND ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))

project (huestream_performance_test C CXX)

set(files
		main.cpp
        hardwaretimer.cpp
        hardwaretimer.h
        scriptwrapper.cpp
        testwrapper.h
        testserver.cpp
        testserver.h
        version.h)

add_executable (huestream_performance_test ${files})
include_directories(
		..
)
target_link_libraries(huestream_performance_test huestream)

copy_libs_to_output(huestream_performance_test huestream)

endif()