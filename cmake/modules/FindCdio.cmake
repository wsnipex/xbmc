#.rst:
# FindCdio
# --------
# Finds the cdio library
#
# This will define the following variables::
#
# CDIO_FOUND - system has cdio
# CDIO_INCLUDE_DIRS - the cdio include directory
# CDIO_LIBRARIES - the cdio libraries

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_CDIO libcdio>=2.1.0 QUIET)
  pkg_check_modules(PC_CDIOPP libcdio++>=2.1.0 QUIET)
endif()

find_path(CDIO_INCLUDE_DIR NAMES cdio/cdio.h
                           PATHS ${PC_CDIO_INCLUDEDIR})

find_library(CDIO_LIBRARY NAMES cdio libcdio
                          PATHS ${PC_CDIO_LIBDIR})

find_path(CDIOPP_INCLUDE_DIR NAMES cdio++/cdio.hpp
                             PATHS ${PC_CDIOPP_INCLUDEDIR} ${CDIO_INCLUDE_DIR})

if(PC_CDIOPP_VERSION AND PC_CDIOPP_VERSION VERSION_EQUAL PC_CDIO_VERSION)
  set(CDIO_VERSION ${PC_CDIO_VERSION})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cdio
                                  REQUIRED_VARS CDIO_LIBRARY CDIO_INCLUDE_DIR
                                  VERSION_VAR CDIO_VERSION)

if(CDIO_FOUND)
  set(CDIO_LIBRARIES ${CDIO_LIBRARY})
  set(CDIO_INCLUDE_DIRS ${CDIO_INCLUDE_DIR})
endif()

mark_as_advanced(CDIO_INCLUDE_DIR CDIOPP_INCLUDE_DIR CDIO_LIBRARY)
