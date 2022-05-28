# FindP8Platform
# -------
# Finds the P8-Platform library
#
# This will define the following variables::
#
# P8PLATFORM_FOUND - system has P8-Platform
# P8PLATFORM_INCLUDE_DIRS - the P8-Platform include directory
# P8PLATFORM_LIBRARIES - the P8-Platform libraries
#
# and the following imported targets::
#
#   P8Platform::P8Platform   - The P8-Platform library

include(cmake/scripts/common/ModuleHelpers.cmake)

# If find_package REQUIRED, check again to make sure any potential versions
# supplied in the call match what we can find/build
if(NOT P8Platform::P8Platform OR P8Platform_FIND_REQUIRED)

  set(MODULE_LC p8-platform)
  SETUP_BUILD_VARS()

  # Search cmake-config. Suitable all platforms
  find_package(p8-platform CONFIG)

  # If a Required version is passed to the find_package call, force build if less
  # than existing found version
  set(FORCE_BUILD OFF)
  if(P8Platform_FIND_VERSION)
    if(p8-platform_VERSION VERSION_LESS ${P8Platform_FIND_VERSION})
      set(FORCE_BUILD ON)
    endif()
  endif()

  if(FORCE_BUILD OR p8-platform_VERSION VERSION_LESS ${${MODULE}_VER})
    set(FORCE_BUILD ON)
    if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
      list(APPEND p8-platform_LIBRARIES "-framework CoreVideo")
    endif()

    set(patches "${CORE_SOURCE_DIR}/tools/depends/target/${MODULE_LC}/001-all-fix-c++17-support.patch"
                "${CORE_SOURCE_DIR}/tools/depends/target/${MODULE_LC}/002-all-fixcmakeinstall.patch"
                "${CORE_SOURCE_DIR}/tools/depends/target/${MODULE_LC}/003-all-cmake_tweakversion.patch")

    generate_patchcommand("${patches}")

    set(CMAKE_ARGS -DBUILD_SHARED_LIBS=OFF)

    # CMAKE_INSTALL_LIBDIR in p8-platform prepends project prefix, so disable sending any
    # install_libdir to generator
    set(P8-PLATFORM_INSTALL_LIBDIR "")

    set(BUILD_NAME lib-${MODULE_LC})

    BUILD_DEP_TARGET()

    set(p8-platform_VERSION ${${MODULE}_VER})
    set(p8-platform_LIBRARY ${${MODULE}_LIBRARY})
    set(p8-platform_INCLUDE_DIRS ${${MODULE}_INCLUDE_DIR})
    set(p8-platform_LIBRARIES "${${MODULE}_LIBRARY}"
                              "${CMAKE_THREAD_LIBS_INIT}")

  else()
    # If Cmake-config didnt find anything, try pkg-config
    if(NOT p8-platform_FOUND)
      if(PKG_CONFIG_FOUND)
        pkg_check_modules(PC_P8PLATFORM p8-platform QUIET)
      endif()

      find_library(p8-platform_LIBRARY NAMES p8-platform
                                       PATHS ${PC_P8PLATFORM_LIBDIR})
      find_path(p8-platform_INCLUDE_DIRS p8-platform/os.h
                                       PATHS ${PC_P8PLATFORM_INCLUDEDIR})

      set(p8-platform_LIBRARIES ${PC_P8PLATFORM_LIBRARIES})
      set(p8-platform_VERSION ${PC_P8PLATFORM_VERSION})
    endif()
  endif()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(P8Platform
                                    REQUIRED_VARS p8-platform_LIBRARY p8-platform_INCLUDE_DIRS p8-platform_LIBRARIES
                                    VERSION_VAR p8-platform_VERSION)

  if(P8PLATFORM_FOUND)
    set(P8PLATFORM_LIBRARY ${p8-platform_LIBRARY})
    set(P8PLATFORM_LIBRARIES "${p8-platform_LIBRARIES}")
    set(P8PLATFORM_INCLUDE_DIRS "${p8-platform_INCLUDE_DIRS}")

    if(NOT TARGET P8Platform::P8Platform OR FORCE_BUILD)
      if(NOT TARGET P8Platform::P8Platform)
        add_library(P8Platform::P8Platform UNKNOWN IMPORTED)
      endif()

      set_target_properties(P8Platform::P8Platform PROPERTIES
                                                   IMPORTED_LOCATION "${P8PLATFORM_LIBRARY}"
                                                   INTERFACE_LINK_LIBRARIES "P8PLATFORM_LIBRARIES"
                                                   INTERFACE_INCLUDE_DIRECTORIES "${P8PLATFORM_INCLUDE_DIRS}")
      if(TARGET lib-p8-platform)
        add_dependencies(P8Platform::P8Platform lib-p8-platform)
        set_target_properties(P8Platform::P8Platform PROPERTIES LIB_BUILD ON)
      endif()
    endif()
  else()
    if(P8PLATFORM_FIND_REQUIRED)
      message(FATAL_ERROR "P8-PLATFORM not found.")
    endif()
  endif()

  mark_as_advanced(p8-platform_LIBRARY p8-platform_LIBRARIES p8-platform_INCLUDE_DIRS)
endif()
