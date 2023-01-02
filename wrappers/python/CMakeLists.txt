#============================================
# python wrapper
#============================================

#------------------------------------------
# Create directory structure
#------------------------------------------
set(python_src_dir ${CMAKE_BINARY_DIR}/bin/wrapper/python CACHE INTERNAL "")
set(python_bin_dir ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "")
file(MAKE_DIRECTORY ${python_src_dir})
file(MAKE_DIRECTORY ${python_bin_dir})

#------------------------------------------
# Include Python
#------------------------------------------
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_PATH})

#------------------------------------------
# Configure the module to swig
#------------------------------------------
# Assumes the following exists:
#     * ../<swig_module_name>.i

set(swig_module_name huestream)
set(swig_target HueStreamWrapper)
set(swig_interface_file ../${swig_module_name}.i)
set(swig_module_dependencies huestream support bridge_discovery)

if (WIN32)
    set(PYLIB_SUFFIX .pyd)
else()
    set(PYLIB_SUFFIX .so)
endif()

#------------------------------------------
# Do the swig action
#------------------------------------------
set(CMAKE_SWIG_FLAGS "")
set_source_files_properties(${swig_interface_file} PROPERTIES CPLUSPLUS ON)

# Should be swig_add_library, but cmake of AS is too old.
swig_add_module(${swig_target} python ${swig_interface_file})
swig_link_libraries(${swig_target} ${swig_module_dependencies} ${PYTHON_LIBRARIES})
set(swig_target_real_name ${SWIG_MODULE_${swig_target}_REAL_NAME})

# add dependency on swig_build if swig is being built
if (BUILD_SWIG)
    add_dependencies(${swig_target_real_name} swig_build)
endif()

# Files to install with Python
set(PYTHON_INSTALL_FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${swig_target}.py
        ${python_bin_dir}/${swig_target_real_name}${PYLIB_SUFFIX})

# Configure setup.py and copy to output directory
set(SETUP_PY_IN ${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in)
set(SETUP_PY_OUT ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
configure_file(${SETUP_PY_IN} ${SETUP_PY_OUT})


# Install target to call setup.py
add_custom_target(install-python
        DEPENDS ${swig_target_real_name}
        COMMAND python ${SETUP_PY_OUT} install)
