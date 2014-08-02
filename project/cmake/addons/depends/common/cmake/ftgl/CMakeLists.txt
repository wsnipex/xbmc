project(ftgl)

cmake_minimum_required(VERSION 2.8)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

### setup all the necessary paths
if(NOT XBMCROOT)
  set(XBMCROOT ${PROJECT_SOURCE_DIR}/../../../../../../..)
else()
  file(TO_CMAKE_PATH "${XBMCROOT}" XBMCROOT)
endif()
get_filename_component(XBMCROOT "${XBMCROOT}" ABSOLUTE)

if(NOT CMAKE_INSTALL_PREFIX)
  message(FATAL_ERROR "CMAKE_INSTALL_PREFIX (${CMAKE_INSTALL_PREFIX}) is not a valid target directory.")
else()
  file(TO_CMAKE_PATH "${CMAKE_INSTALL_PREFIX}" CMAKE_INSTALL_PREFIX)
endif()
get_filename_component(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" ABSOLUTE)
message(STATUS "INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
include(ExternalProject)

ExternalProject_ADD(ftgl
                    SOURCE_DIR ${PROJECT_SOURCE_DIR}
                    DOWNLOAD_COMMAND ""
                    UPDATE_COMMAND cd unix && autoreconf -vif
                    CONFIGURE_COMMAND cd unix && CXXFLAGS=-fpermissive ./configure --prefix=${CMAKE_INSTALL_PREFIX} --with-pic
                    BUILD_COMMAND cd unix && make
                    INSTALL_COMMAND cd unix && make install
                    BUILD_IN_SOURCE 1)

