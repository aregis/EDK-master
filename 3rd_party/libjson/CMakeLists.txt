cmake_minimum_required(VERSION 3.6)

#
# libjson
#
file(GLOB_RECURSE JSON_SOURCES
        "_internal/Source/[^.]*.cpp" "_internal/Source/[^.]*.h")
file(GLOB_RECURSE UNUSED_JSON_SOURCES
        "_internal/Source/JSONAllocator.cpp"
        "_internal/Source/JSONDebug.cpp"
        "_internal/Source/JSONMemory.cpp"
        "_internal/Source/JSONNode_Mutex.cpp"
        "_internal/Source/JSONPreparse.cpp"
        "_internal/Source/JSONStream.cpp"
        "_internal/Source/libjson.cpp")
list(REMOVE_ITEM JSON_SOURCES ${UNUSED_JSON_SOURCES})

add_library(json STATIC ${JSON_SOURCES})

install(
        TARGETS json
        ARCHIVE DESTINATION lib
        )

install(FILES "libjson.h" "JSONOptions.h"
            DESTINATION ${INSTALL_DESTINATION} )

install(DIRECTORY "_internal"
    DESTINATION ${INSTALL_DESTINATION}
    FILES_MATCHING PATTERN "*.h"
    )