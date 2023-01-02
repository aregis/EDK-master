cmake_minimum_required(VERSION 3.6)

if(ANDROID)	    
	set(PATCH_USER_CONFIG ${EXTERNAL_LIBRARIES_SOURCE_PATH}/boost_user_config.hpp)
else()
	set(PATCH_USER_CONFIG ${EXTERNAL_LIBRARIES_SOURCE_PATH}/boost/src/boost/config/user.hpp)
endif(ANDROID)

# On Mac Ventura using the URL param with ExternalProject_Add cause an error when CMake try to automatically unzip the downloaded Boost archive. However calling it in a separate command does work.
if (${CMAKE_GENERATOR} STREQUAL "Xcode")
    set(BOOST_NO_EXTRACT DOWNLOAD_NO_EXTRACT true)
    set(BOOST_EXTRACT_COMMAND ${CMAKE_COMMAND} -E chdir ${EXTERNAL_LIBRARIES_SOURCE_PATH}/boost/src tar xzf ${EXTERNAL_LIBRARIES_SOURCE_PATH}/boost/src/boost_${boost_VERSION}.zip)
    set(BOOST_MOVE_COMMAND ${CMAKE_COMMAND} -E copy_directory ${EXTERNAL_LIBRARIES_SOURCE_PATH}/boost/src/boost_${boost_VERSION} ${EXTERNAL_LIBRARIES_SOURCE_PATH}/boost/src)
    set(BOOST_DELETE_COMMAND ${CMAKE_COMMAND} -E rm -r ${EXTERNAL_LIBRARIES_SOURCE_PATH}/boost/src/boost_${boost_VERSION})
else()
    set(BOOST_NO_EXTRACT "")	
    set(BOOST_EXTRACT_COMMAND "")
    set(BOOST_MOVE_COMMAND "")
    set(BOOST_DELETE_COMMAND "")
endif(${CMAKE_GENERATOR} STREQUAL "Xcode")

ExternalProject_Add(external_boost
    PREFIX ${EXTERNAL_LIBRARIES_BUILD_PATH}    
	URL "${boost_URL}" ${BOOST_NO_EXTRACT}
    UPDATE_COMMAND ""
    DOWNLOAD_DIR "${EXTERNAL_LIBRARIES_SOURCE_PATH}/boost/src"
    SOURCE_DIR "${EXTERNAL_LIBRARIES_SOURCE_PATH}/boost/src"
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy "${EXTERNAL_LIBRARIES_SOURCE_PATH}/CMakeLists.EDK.Boost.txt" "${EXTERNAL_LIBRARIES_SOURCE_PATH}/boost/src/CMakeLists.txt"	
	COMMAND ${CMAKE_COMMAND} -E copy "${PATCH_USER_CONFIG}" "${EXTERNAL_LIBRARIES_SOURCE_PATH}/boost/src/boost/config/user.hpp"
    CMAKE_ARGS ${COMMON_ARGS} -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD} -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -DINSTALL_DESTINATION=include/boost -DCMAKE_PREFIX_PATH=${EXTERNAL_LIBRARIES_INSTALL_PATH} -DCMAKE_INSTALL_PREFIX=${EXTERNAL_LIBRARIES_INSTALL_PATH}
    LIST_SEPARATOR ^^
    )
    
ExternalProject_Add_Step(external_boost extract
    COMMAND ${BOOST_EXTRACT_COMMAND}
    COMMAND ${BOOST_MOVE_COMMAND}
    COMMAND ${BOOST_DELETE_COMMAND}
    DEPENDEES download
    DEPENDERS patch
    )

add_library(boost INTERFACE)
target_include_directories(boost INTERFACE ${EXTERNAL_LIBRARIES_INSTALL_PATH}/include)
add_dependencies(boost external_boost)
