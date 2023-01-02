if (BUILD_SWIG)
    message(STATUS "Building SWIG")
    include(ExternalProjectSwig)
else()
    find_package(SWIG REQUIRED)
endif()

include(UseSWIG)

#mac not supported yet and RTTI needed
if(BUILD_WITH_RTTI)
    if(NOT ANDROID)
        add_subdirectory(csharp)
    endif()    
    add_subdirectory(java)
    #add_subdirectory(python) #not actively supported
endif()
