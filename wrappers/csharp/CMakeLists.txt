#------------------------------------------------------------------------------
# csharp wrapper
#------------------------------------------------------------------------------
set(csharp_bin_dir                 ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "")
set(csharp_obj_dir                 ${CMAKE_BINARY_DIR}/obj CACHE INTERNAL "")
set(csharp_dst_project_dir         ${csharp_bin_dir}/wrapper/csharp CACHE INTERNAL "")

set(csharp_src_project_integrationtests     ${CMAKE_CURRENT_LIST_DIR}/huestream_csharp_integrationtests)
set(csharp_src_project_managed     ${CMAKE_CURRENT_LIST_DIR}/huestream_csharp_managed)
set(csharp_src_project_example_wpf ${CMAKE_CURRENT_LIST_DIR}/../../examples/huestream_csharp_example_wpf)

set(csharp_dst_project_managed     ${csharp_dst_project_dir}/huestream_csharp_managed)
set(csharp_dst_project_example_wpf ${csharp_dst_project_dir}/huestream_csharp_example_wpf)
set(csharp_dst_generated           ${csharp_dst_project_managed}/generated CACHE INTERNAL "")

#------------------------------------------------------------------------------
# Create directory structure
#------------------------------------------------------------------------------
file(MAKE_DIRECTORY ${csharp_bin_dir})
file(MAKE_DIRECTORY ${csharp_obj_dir})
file(MAKE_DIRECTORY ${csharp_dst_project_managed})
file(MAKE_DIRECTORY ${csharp_dst_project_example_wpf})
file(MAKE_DIRECTORY ${csharp_dst_generated})

#------------------------------------------------------------------------------
# Copy integration tests solution files to bin dir
#------------------------------------------------------------------------------
file(COPY ${csharp_src_project_integrationtests}     DESTINATION ${csharp_dst_project_dir})

#------------------------------------------------------------------------------
# Copy project (template) files to bin dir
#------------------------------------------------------------------------------
file(COPY ${csharp_src_project_managed}     DESTINATION ${csharp_dst_project_dir})
file(COPY ${csharp_src_project_example_wpf} DESTINATION ${csharp_dst_project_dir})

#------------------------------------------------------------------------------
# Configure swig
#------------------------------------------------------------------------------
set(CMAKE_SWIG_OUTDIR ${csharp_dst_generated})

#------------------------------------------------------------------------------
# Configure the module to swig
#------------------------------------------------------------------------------
set(swig_module_name huestream)
set(swig_module_namespace huestream)

set(swig_target ${swig_module_name}_csharp_native)
set(swig_interface_file ${CMAKE_CURRENT_LIST_DIR}/../${swig_module_name}.i)
set(swig_module_dependencies huestream support bridge_discovery)

#------------------------------------------------------------------------------
# Do the swig action
#------------------------------------------------------------------------------
set_source_files_properties(${swig_interface_file} PROPERTIES CPLUSPLUS ON)
set_source_files_properties(${swig_interface_file} PROPERTIES SWIG_FLAGS "-namespace;${swig_module_namespace};-v")

# Should be swig_add_library, but cmake of jenkins/AS is too old.
swig_add_module(${swig_target} csharp ${swig_interface_file})
#swig_add_library(${swig_target}
#                 LANGUAGE csharp
#                 SOURCES ${swig_interface_file}
#                 )
swig_link_libraries(${swig_target} ${swig_module_dependencies})

#now the real target name is clear
set(swig_real_target_name ${SWIG_MODULE_${swig_target}_REAL_NAME})
set_target_properties(${swig_real_target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${csharp_bin_dir})
IF(APPLE)
	SET_TARGET_PROPERTIES(${swig_real_target_name} PROPERTIES SUFFIX ".dylib")
ENDIF(APPLE)

# shared lib settings
generate_export_header(${swig_target})
set_target_properties(${swig_target} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

# add dependency on swig_build if swig is being built
if (BUILD_SWIG)
    add_dependencies(${SWIG_MODULE_${swig_target}_REAL_NAME} swig_build)
endif()

if(${CMAKE_SIZEOF_VOID_P} EQUAL "8")
    set(csharp_platform_name "x64")
else()
    set(csharp_platform_name "x86")
endif()

string(REPLACE "/" "\\" csharp_bin_dir_dos_style "${csharp_bin_dir}")
string(REPLACE "/" "\\" csharp_src_dir_dos_style "${csharp_dst_generated}")
string(REPLACE "/" "\\" csharp_obj_dir_dos_style "${csharp_obj_dir}")

#------------------------------------------------------------------------------
# Build C# lib
#------------------------------------------------------------------------------
if (NOT MSVC)
	# directly build CSharp, not using Visual studio specific C# projects
	find_package(CSharp)

	if(NOT CSHARP_FOUND)
		message(WARNING "C# compiler not found, unable to generate C# binding library")
		return()
	endif()

	include(${CSHARP_USE_FILE})
	set(CSHARP_SOURCE_DIRECTORY ${csharp_dst_generated})
	set(CSHARP_BINARY_DIRECTORY ${csharp_bin_dir})

	csharp_add_library(${swig_module_name}_csharp_managed ${csharp_dst_generated}/*.cs)
	add_dependencies(${swig_module_name}_csharp_managed ${swig_real_target_name})
else()
	# add visual studio real C# projects, instead of C++ projects building C# code
	
	# project huestream_csharp_managed (depends on )
	set(csdepencency ${swig_real_target_name})
	set(csname ${swig_module_name}_csharp_managed)
	set(csproject_template_name ${csharp_dst_project_dir}/${csname}/${csname}.template.csproj)
	set(csproject_name          ${csharp_dst_project_dir}/${csname}/${csname}.csproj)
	configure_file( ${csproject_template_name} ${csproject_name})
	include_external_msproject( ${csname} ${csproject_name} TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
	add_dependencies(${csname} ${csdepencency}) # NOTE: dependency setting does not seem to work reliable, also not when set as an option with include_external_msproject

	# project huestream_csharp_example_wpf (depends on huestream_csharp_managed)
	set(csdepencency ${csname})
	set(csname ${swig_module_name}_csharp_example_wpf)
	set(csproject_template_name ${csharp_dst_project_dir}/${csname}/${csname}.template.csproj)
	set(csproject_name          ${csharp_dst_project_dir}/${csname}/${csname}.csproj)
	configure_file( ${csproject_template_name} ${csproject_name})
	include_external_msproject(${csname} ${csproject_name} TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
	add_dependencies(${csname} ${csdepencency}) # NOTE: dependency setting does not seem to work reliable, also not when set as an option with include_external_msproject
endif()

