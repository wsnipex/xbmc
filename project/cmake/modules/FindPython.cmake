# - Try to find python
# Once done this will define
#
# PYTHON_FOUND - system has PYTHON
# PYTHON_INCLUDE_DIRS - the python include directory
# PYTHON_LIBRARIES - The python libraries

if(PKG_CONFIG_FOUND AND NOT CMAKE_CROSSCOMPILING)
  pkg_check_modules (PYTHON python)
endif()

if(NOT PYTHON_FOUND)
  if(CMAKE_CROSSCOMPILING)
    find_program(PYTHON_EXECUTABLE python ONLY_CMAKE_FIND_ROOT_PATH)
    find_library(PYTHON_LIBRARY NAMES python2.6 python2.7)
    find_path(PYTHON_INCLUDE_DIRS NAMES Python.h PATHS ${DEPENDS_PATH}/include/python2.6 ${DEPENDS_PATH}/include/python2.7)
    set(PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_DIRS} CACHE PATH "python include dir" FORCE)

    find_library(FFI_LIBRARY ffi)
    find_library(EXPAT_LIBRARY expat)
    find_library(INTL_LIBRARY intl)

    set(PYTHON_LIBRARIES ${PYTHON_LIBRARY} ${FFI_LIBRARY} ${EXPAT_LIBRARY} ${INTL_LIBRARY} -lpthread -ldl -lutil
        CACHE INTERNAL "python libraries" FORCE)
  else()
    find_package(PythonLibs)
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Python DEFAULT_MSG PYTHON_INCLUDE_DIRS PYTHON_LIBRARIES)

mark_as_advanced(PYTHON_INCLUDE_DIRS PYTHON_LIBRARIES PYTHON_LDFLAGS)
