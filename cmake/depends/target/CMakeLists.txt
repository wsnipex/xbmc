cmake_minimum_required(VERSION 3.4)
project(kodi-target-depends LANGUAGES CXX C ASM)

include(ExternalProject)

if(NOT DEPENDS_DEFINITION_DIR)
  set(DEPENDS_DEFINITION_DIR ${PROJECT_SOURCE_DIR}/depends)
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

#if(NOT APP_LIB_DIR)
#  set(APP_LIB_DIR "${ADDON_DEPENDS_PATH}/lib/kodi")
#else()
#  file(TO_CMAKE_PATH "${APP_LIB_DIR}" APP_LIB_DIR)
#endif()


list(APPEND CMAKE_MODULE_PATH ${CORE_SOURCE_DIR}/cmake/modules)
# make sure CMAKE_PREFIX_PATH is set
if(NOT CMAKE_PREFIX_PATH)
  set(CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}")
else()
  file(TO_CMAKE_PATH "${CMAKE_PREFIX_PATH}" CMAKE_PREFIX_PATH)
  list(APPEND CMAKE_PREFIX_PATH "${ADDON_DEPENDS_PATH}")
endif()


