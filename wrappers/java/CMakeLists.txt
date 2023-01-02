#============================================
# java wrapper
#============================================

#------------------------------------------
# Create directory structure
#------------------------------------------
set(java_pkg_dir com/lighting/huestream CACHE INTERNAL "")
set(java_src_dir ${CMAKE_BINARY_DIR}/bin/wrapper/java/src CACHE INTERNAL "")
set(java_cls_dir ${CMAKE_BINARY_DIR}/wrappers/java/classes CACHE INTERNAL "")
set(java_bin_dir ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "")
file(MAKE_DIRECTORY ${java_src_dir}/${java_pkg_dir})
file(MAKE_DIRECTORY ${java_cls_dir}/${java_pkg_dir})
file(MAKE_DIRECTORY ${java_bin_dir})

#------------------------------------------
# find JNI
#------------------------------------------
find_package(Java REQUIRED)
include(UseJava)

if(NOT ANDROID)
    find_package(JNI REQUIRED)
    include_directories(${JNI_INCLUDE_DIRS})
endif()

#------------------------------------------
# configure swig
#------------------------------------------
set(CMAKE_SWIG_OUTDIR ${java_src_dir}/${java_pkg_dir})

#------------------------------------------
# Configure the module to swig
#------------------------------------------
# Assumes the following exists:
#     * ../<swig_module_name>.i

set(swig_module_name huestream)
set(swig_module_package com.lighting.huestream)
set(swig_target ${swig_module_name}_java_native)
set(swig_interface_file ../${swig_module_name}.i)
set(swig_module_dependencies huestream support bridge_discovery)
set(swig_jar_name lib${swig_module_name}_java_managed)

if (ANDROID)
    list(APPEND swig_module_dependencies "log")
endif()

#------------------------------------------
# Do the swig action
#------------------------------------------
set_source_files_properties(${swig_interface_file} PROPERTIES CPLUSPLUS ON)
set_source_files_properties(${swig_interface_file} PROPERTIES SWIG_FLAGS "-package;${swig_module_package}")

# Should be swig_add_library, but cmake of AS is too old.
swig_add_module(${swig_target} java ${swig_interface_file})
swig_link_libraries(${swig_target} ${swig_module_dependencies})
set(swig_target_real_name ${SWIG_MODULE_${swig_target}_REAL_NAME})
set_target_properties(${swig_target_real_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${java_bin_dir})


# shared lib settings
generate_export_header(${swig_target})
set_target_properties(${swig_target} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

# add dependency on swig_build if swig is being built
if (BUILD_SWIG)
    add_dependencies(${SWIG_MODULE_${swig_target}_REAL_NAME} swig_build)
endif()


if(${CMAKE_SIZEOF_VOID_P} EQUAL "8")
  set(java_platform_name "x64")
else()
  set(java_platform_name "x86")
endif()

string(REPLACE "/" "\\" java_bin_dir_dos_style "${java_bin_dir}")
string(REPLACE "/" "\\" DOS_STYLE_SOURCE_DIR "${java_src_dir}/${java_pkg_dir}")

#------------------------------------------
# Build java lib
#------------------------------------------
if(ANDROID)
    set(extra_javac_options "-target" "1.7" "-source" "1.7")
endif()

if (ANDROID)
ADD_CUSTOM_COMMAND(TARGET ${swig_target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/src ${java_src_dir}/
    )
endif()

ADD_CUSTOM_COMMAND(TARGET ${swig_target} POST_BUILD
    COMMAND "${Java_JAVAC_EXECUTABLE}" ${extra_javac_options} -d ${java_cls_dir} ${java_src_dir}/${java_pkg_dir}/*.java
    COMMAND "${Java_JAR_EXECUTABLE}" -cfM ${java_bin_dir}/${swig_jar_name}.jar -C ${java_cls_dir} .
    )

#--------------------------------------------------------------------------#
# Copy artifacts to aar directory on Android and generate the aar archive. #
#--------------------------------------------------------------------------#
if (ANDROID)
ADD_CUSTOM_COMMAND(TARGET ${swig_target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/android-edk-aar ${CMAKE_BINARY_DIR}/android-edk-aar/
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/android-edk-aar/hue-edk/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${java_src_dir}/ ${CMAKE_BINARY_DIR}/android-edk-aar/hue-edk/src/main/java/src/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${swig_target_real_name}> ${CMAKE_BINARY_DIR}/android-edk-aar/hue-edk/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/    
    )
    if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
        if (NOT ${CMAKE_C_FLAGS_DEBUG} EQUAL "")
            set(arr_configuration "Debug")
            set(arr_output_file_name "hue-edk-debug.aar")
        else()
            set(arr_configuration "Release")
            set(arr_output_file_name "hue-edk-release.aar")
        endif()
        ADD_CUSTOM_COMMAND(TARGET ${swig_target} POST_BUILD
        COMMAND "cd" ${CMAKE_BINARY_DIR}/android-edk-aar
        COMMAND "gradlew.bat" assemble${arr_configuration}
        )
        ADD_CUSTOM_COMMAND(TARGET ${swig_target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/android-edk-aar/hue-edk/build/outputs/aar/${arr_output_file_name} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
        )
    endif()
endif()

#---------------------------------------------------#
# Copy artifacts to IntegrationTests project setup--#
#---------------------------------------------------#
if(NOT ANDROID)
    set(integrationtests_needed_artifacts_dir ${CMAKE_CURRENT_LIST_DIR}/integration_tests/lib)
    set(native_lib_name "native")
else()
    set(integrationtests_needed_artifacts_dir ${CMAKE_CURRENT_LIST_DIR}/android_integration_tests/app/lib)
    set(native_lib_name "x86")
endif()

ADD_CUSTOM_COMMAND(TARGET ${swig_target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${integrationtests_needed_artifacts_dir}/${native_lib_name}
    COMMAND ${CMAKE_COMMAND} -E copy ${java_bin_dir}/${swig_jar_name}.jar ${integrationtests_needed_artifacts_dir}/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${swig_target_real_name}> ${integrationtests_needed_artifacts_dir}/${native_lib_name}/
)
