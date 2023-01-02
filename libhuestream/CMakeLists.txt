list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

#============================================
# support
#============================================

if (NOT HTTP_CLIENT)
    set(HTTP_CLIENT curl)
endif(NOT HTTP_CLIENT)

if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/support)
    add_subdirectory(support)
endif()

#============================================
# bridgediscovery
#============================================
if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bridgediscovery)
    add_subdirectory(bridgediscovery)
endif()

#============================================
# huestream
#============================================
add_subdirectory(huestream)

if (BUILD_TEST)
    if(XCODE)
        message("EDK Tests disabled because of lack of support for console apps on mobile platforms")
    else(XCODE)
        add_subdirectory(test)
    endif(XCODE)
endif ()
