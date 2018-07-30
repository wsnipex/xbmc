# Copyright 2014 Stefan.Eilemann@epfl.ch
# Copyright 2014 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Find the flatbuffers schema compiler
#
# Output Variables:
#
# FLATBUFFERS_FOUND - system has flatbuffer compiler and headers
# FLATBUFFERS_FLATC_EXECUTABLE - the flatc compiler executable
# FLATBUFFERS_INCLUDE_DIRS - the flatbuffers include directory
#
# Provides:
#
# FLATBUFFERS_GENERATE_C_HEADERS(Name <files>) creates the C++ headers
#   for the given flatbuffer schema files.
#   Returns the header files in ${Name}_OUTPUTS

if(ENABLE_INTERNAL_FLATBUFFERS)
  include(ExternalProject)
  file(STRINGS ${CMAKE_SOURCE_DIR}/tools/depends/native/flatbuffers-native/Makefile VER REGEX "^[ ]*VERSION[ ]*=.+$")
  string(REGEX MATCH "VERSION=[^ ]*" FLATBUFFERS_VER "${VER}")

  # allow user to override the download URL with a local tarball
  # needed for offline build envs
  if(FLATBUFFERS_URL)
    get_filename_component(FLATBUFFERS_URL "${FLATBUFFERS_URL}" ABSOLUTE)
  else()
    #set(FLATBUFFERS_URL http://mirrors.kodi.tv/build-deps/sources/flatbuffers-${FLATBUFFERS_VER}.tar.gz)
    set(FLATBUFFERS_URL https://github.com/google/flatbuffers/archive/v${FLATBUFFERS_VER}.tar.gz)
  endif()
  if(VERBOSE)
    message(STATUS "FLATBUFFERS_URL: ${FLATBUFFERS_URL}")
  endif()

  set(FLATBUFFERS_FLATC_EXECUTABLE ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/bin/flatc)
  set(FLATBUFFERS_INCLUDE_DIR ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/include)

  externalproject_add(flatbuffers
                      URL ${FLATBUFFERS_URL}
                      DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/download
                      PREFIX ${CORE_BUILD_DIR}/flatbuffers
                      CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
                                 -DCMAKE_BUILD_TYPE=Release
                                 -DFLATBUFFERS_CODE_COVERAGE=OFF
                                 -DFLATBUFFERS_BUILD_TESTS=OFF
                                 -DFLATBUFFERS_INSTALL=ON
                                 -DFLATBUFFERS_BUILD_FLATLIB=OFF
                                 -DFLATBUFFERS_BUILD_FLATC=ON
                                 -DFLATBUFFERS_BUILD_FLATHASH=OFF
                                 -DFLATBUFFERS_BUILD_GRPCTEST=OFF
                                 -DFLATBUFFERS_BUILD_SHAREDLIB=OFF
                                 "${EXTRA_ARGS}"
                      BUILD_BYPRODUCTS ${FLATBUFFERS_FLATC_EXECUTABLE})
  set_target_properties(flatbuffers PROPERTIES FOLDER "External Projects")
else()
  find_program(FLATBUFFERS_FLATC_EXECUTABLE NAMES flatc)
  find_path(FLATBUFFERS_INCLUDE_DIR NAMES flatbuffers/flatbuffers.h)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(flatbuffers
  DEFAULT_MSG FLATBUFFERS_FLATC_EXECUTABLE FLATBUFFERS_INCLUDE_DIR)

  function(FLATBUFFERS_GENERATE_C_HEADERS Name)
    set(FLATC_OUTPUTS)
    foreach(FILE ${ARGN})
      get_filename_component(FLATC_OUTPUT ${FILE} NAME_WE)
      set(FLATC_OUTPUT
        "${CMAKE_CURRENT_BINARY_DIR}/${FLATC_OUTPUT}_generated.h")
      list(APPEND FLATC_OUTPUTS ${FLATC_OUTPUT})

      add_custom_command(OUTPUT ${FLATC_OUTPUT}
        COMMAND ${FLATBUFFERS_FLATC_EXECUTABLE}
        ARGS -c -o "${CMAKE_CURRENT_BINARY_DIR}/" ${FILE}
        DEPENDS ${FILE}
        COMMENT "Building C++ header for ${FILE}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endforeach()
    set(${Name}_OUTPUTS ${FLATC_OUTPUTS} PARENT_SCOPE)
  endfunction()

set(FLATBUFFERS_INCLUDE_DIRS ${FLATBUFFERS_INCLUDE_DIR})
include_directories(${CMAKE_BINARY_DIR})

mark_as_advanced(FLATBUFFERS_INCLUDE_DIR)
