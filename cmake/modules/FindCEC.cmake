#.rst:
# FindCEC
# -------
# Finds the libCEC library
#
# This will define the following variables::
#
# CEC_FOUND - system has libCEC
# CEC_INCLUDE_DIRS - the libCEC include directory
# CEC_LIBRARIES - the libCEC libraries
# CEC_DEFINITIONS - the libCEC compile definitions
#
# and the following imported targets::
#
#   CEC::CEC   - The libCEC library

include(cmake/scripts/common/ModuleHelpers.cmake)

if(ENABLE_INTERNAL_CEC)

  # Check for dependencies - Must be done before SETUP_BUILD_VARS
  get_libversion_data("p8-platform" "target")
  find_package(P8Platform ${LIB_P8-PLATFORM_VER} MODULE QUIET REQUIRED)

  # Check if we want to force a build due to a dependency rebuild
  get_property(LIB_FORCE_REBUILD TARGET P8Platform::P8Platform PROPERTY LIB_BUILD)

  set(MODULE_LC cec)

  SETUP_BUILD_VARS()

  # Check for existing libcec. If version >= LIBCEC-VERSION file version, dont build
  find_package(libcec CONFIG)

  if(libcec_VERSION VERSION_LESS ${${MODULE}_VER} OR LIB_FORCE_REBUILD)

    set(CEC_VERSION ${${MODULE}_VER})

    set(patches "${CORE_SOURCE_DIR}/tools/depends/target/${MODULE_LC}/001-all-enableclient.patch"
                "${CORE_SOURCE_DIR}/tools/depends/target/${MODULE_LC}/002-all-libcecinstallprefix.patch"
                "${CORE_SOURCE_DIR}/tools/depends/target/${MODULE_LC}/remove_git_info.patch"
                "${CORE_SOURCE_DIR}/tools/depends/target/${MODULE_LC}/005-all-cmake-version.patch")

    if(WIN32 OR WINDOWS_STORE)
      list(APPEND patches "${CORE_SOURCE_DIR}/tools/depends/target/${MODULE_LC}/003-win-remove_32bit_timet.patch")
      list(APPEND patches "${CORE_SOURCE_DIR}/tools/depends/target/${MODULE_LC}/004-win-pdbstatic.patch")
    endif()

    generate_patchcommand("${patches}")

    set(CMAKE_ARGS -DBUILD_SHARED_LIBS=ON
                   -DSKIP_PYTHON_WRAPPER=ON
                   -DCMAKE_PLATFORM_NO_VERSIONED_SONAME=ON)

    if(WIN32 AND ARCH STREQUAL "x64")
      # Disable _USE_32BIT_TIME_T for x64 win target
      list(APPEND CMAKE_ARGS -DWIN64=ON)
    endif()

    if(CORE_SYSTEM_NAME STREQUAL "osx")
      set(CEC_BYPRODUCT_EXTENSION "dylib")
    endif()

    BUILD_DEP_TARGET()

    if(CORE_SYSTEM_NAME STREQUAL "osx")
      find_program(INSTALL_NAME_TOOL NAMES install_name_tool)
      add_custom_command(TARGET cec POST_BUILD
                                    COMMAND ${INSTALL_NAME_TOOL} -id ${CEC_LIBRARY} ${CEC_LIBRARY})
    endif()

    add_dependencies(cec P8Platform::P8Platform)
  else()
    # Existing internal lib found. Setup LIBRARY/INCLUDE_DIR
    find_library(CEC_LIBRARY NAMES cec
                             PATHS ${DEPENDS_PATH}/lib)
    find_path(CEC_INCLUDE_DIR NAMES libcec/cec.h libCEC/CEC.h
                              PATHS ${DEPENDS_PATH}/include)
  endif()
else()
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_CEC libcec QUIET)
  endif()

  find_path(CEC_INCLUDE_DIR NAMES libcec/cec.h libCEC/CEC.h
                            PATHS ${PC_CEC_INCLUDEDIR})

  if(PC_CEC_VERSION)
    set(CEC_VERSION ${PC_CEC_VERSION})
  elseif(CEC_INCLUDE_DIR AND EXISTS "${CEC_INCLUDE_DIR}/libcec/version.h")
    file(STRINGS "${CEC_INCLUDE_DIR}/libcec/version.h" cec_version_str REGEX "^[\t ]+LIBCEC_VERSION_TO_UINT\\(.*\\)")
    string(REGEX REPLACE "^[\t ]+LIBCEC_VERSION_TO_UINT\\(([0-9]+), ([0-9]+), ([0-9]+)\\)" "\\1.\\2.\\3" CEC_VERSION "${cec_version_str}")
    unset(cec_version_str)
  endif()

  if(NOT CEC_FIND_VERSION)
    set(CEC_FIND_VERSION 4.0.0)
  endif()

  find_library(CEC_LIBRARY NAMES cec
                           PATHS ${PC_CEC_LIBDIR})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CEC
                                  REQUIRED_VARS CEC_LIBRARY CEC_INCLUDE_DIR
                                  VERSION_VAR CEC_VERSION)

if(CEC_FOUND)
  set(CEC_LIBRARIES ${CEC_LIBRARY})
  set(CEC_INCLUDE_DIRS ${CEC_INCLUDE_DIR})
  set(CEC_DEFINITIONS -DHAVE_LIBCEC=1)

  if(NOT TARGET CEC::CEC)
    add_library(CEC::CEC UNKNOWN IMPORTED)
    if(CEC_LIBRARY)
      set_target_properties(CEC::CEC PROPERTIES
                                     IMPORTED_LOCATION "${CEC_LIBRARY}")
    endif()
    set_target_properties(CEC::CEC PROPERTIES
                                   INTERFACE_INCLUDE_DIRECTORIES "${CEC_INCLUDE_DIR}"
                                   INTERFACE_COMPILE_DEFINITIONS HAVE_LIBCEC=1)
  endif()
  if(TARGET cec)
    add_dependencies(CEC::CEC cec)
  endif()
  set_property(GLOBAL APPEND PROPERTY INTERNAL_DEPS_PROP CEC::CEC)
endif()

mark_as_advanced(CEC_INCLUDE_DIR CEC_LIBRARY)
