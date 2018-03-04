cmake_minimum_required(VERSION 3.4)
project(kodi-native-depends LANGUAGES CXX C ASM)

include(ExternalProject)

set(CORE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
string(TOLOWER ${CORE_SYSTEM_NAME} CORE_SYSTEM_NAME)

if(NOT DEPENDS_DEFINITION_DIR)
  set(DEPENDS_DEFINITION_DIR ${PROJECT_SOURCE_DIR}/native-depends)
else()
  file(TO_CMAKE_PATH "${DEPENDS_DEFINITION_DIR}" DEPENDS_DEFINITION_DIR)
endif()
get_filename_component(DEPENDS_DEFINITION_DIR "${DEPENDS_DEFINITION_DIR}" ABSOLUTE)

if(DEPENDS_SRC_PREFIX)
  if(NOT IS_ABSOLUTE ${DEPENDS_SRC_PREFIX})
    get_filename_component(DEPENDS_SRC_PREFIX "${CMAKE_BINARY_DIR}/${DEPENDS_SRC_PREFIX}" ABSOLUTE)
  endif()
  message(STATUS "Overriding depends source directory prefix: ${DEPENDS_SRC_PREFIX}")
endif()

set(OUTPUT_DIR ${CMAKE_INSTALL_PREFIX})
set(DEFAULT_INSTALL_COMMAND INSTALL_COMMAND "")

list(APPEND CMAKE_MODULE_PATH ${CORE_SOURCE_DIR}/cmake/modules)
# make sure CMAKE_PREFIX_PATH is set
if(NOT CMAKE_PREFIX_PATH)
  set(CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}")
else()
  file(TO_CMAKE_PATH "${CMAKE_PREFIX_PATH}" CMAKE_PREFIX_PATH)
  list(APPEND CMAKE_PREFIX_PATH "${ADDON_DEPENDS_PATH}")
endif()


find_package(CCache)

set(CMAKE_C_FLAGS -I${CMAKE_INSTALL_PREFIX}/include ${CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS -I${CMAKE_INSTALL_PREFIX}/include ${CMAKE_CXX_FLAGS})
set(CMAKE_CPP_FLAGS -I${CMAKE_INSTALL_PREFIX}/include ${CMAKE_CPP_FLAGS})
set(CMAKE_EXE_LINKER_FLAGS -L${CMAKE_INSTALL_PREFIX}/lib)
if(ENABLE_CCACHE AND CCACHE_FOUND)
  set(CC "${CCACHE_PROGRAM} ${CMAKE_C_COMPILER}")
  set(CXX "${CCACHE_PROGRAM} ${CMAKE_CXX_COMPILER}")
else()
  set(CC ${CMAKE_C_COMPILER})
  set(CXX ${CMAKE_CXX_COMPILER})
endif()
set(LD ${CMAKE_LINKER})
set(AR ${CMAKE_AR})
set(AS ${CMAKE_AS})
set(NM ${CMAKE_NM})
set(RANLIB ${CMAKE_RANLIB})
set(STRIP ${CMAKE_STRIP})
#set(READELF ${})
#set(OBJDUMP ${})
set(NATIVE_PKG_CONFIG_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig:${CMAKE_INSTALL_PREFIX}/share/pkgconfig")
set(CFLAGS ${CMAKE_C_FLAGS})
set(CFLAGS ${CMAKE_CXX_FLAGS})
set(CPPFLAGS ${CMAKE_CPP_FLAGS})
set(LDFLAGS ${CMAKE_EXE_LINKER_FLAGS})


configure_file(${PROJECT_SOURCE_DIR}/config.site.native.in ${CMAKE_INSTALL_PREFIX}/share/config.site @ONLY)

### get and build all native dependencies
#include(${CORE_SOURCE_DIR}/cmake/scripts/common/DownloadDepends.cmake)
include(${CORE_SOURCE_DIR}/cmake/scripts/common/HandleDepends.cmake)
add_addon_depends(kodi-native-depends ${DEPENDS_DEFINITION_DIR})
