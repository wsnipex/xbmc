project(xbmc-addons)

cmake_minimum_required(VERSION 2.8)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(ExternalProject)

### setup all the necessary paths
if(NOT XBMCROOT)
  set(XBMCROOT ${PROJECT_SOURCE_DIR}/../../..)
else()
  file(TO_CMAKE_PATH "${XBMCROOT}" XBMCROOT)
endif()
get_filename_component(XBMCROOT "${XBMCROOT}" ABSOLUTE)

# there's something odd going on with passing -Dprefix so we use -DPREFIX but in here we use ${prefix}
if(NOT PREFIX)
  set(prefix "${CMAKE_BINARY_DIR}/addons")
else()
  set(prefix "${PREFIX}")
  file(TO_CMAKE_PATH "${prefix}" prefix)
endif()
get_filename_component(prefix "${prefix}" ABSOLUTE)

list(APPEND CMAKE_PREFIX_PATH ${prefix})
set(PREFIX_INCLUDE_DIR ${prefix}/include)
set(PREFIX_LIB_DIR ${prefix}/lib)

if(NOT OUTPUT_DIR)
  set(OUTPUT_DIR ${prefix}/addons)
endif()
list(APPEND CMAKE_PREFIX_PATH ${OUTPUT_DIR})

### copy all the addon binding header files to include/xbmc
# make sure include/xbmc exists and is empty
set(XBMC_INCLUDE_DIR ${PREFIX_INCLUDE_DIR}/xbmc)
set(XBMC_LIB_DIR ${PREFIX_LIB_DIR}/xbmc)
if(NOT EXISTS "${XBMC_INCLUDE_DIR}/")
  file(MAKE_DIRECTORY ${XBMC_INCLUDE_DIR})
endif()
if(NOT EXISTS "${XBMC_LIB_DIR}/")
  file(MAKE_DIRECTORY ${XBMC_INCLUDE_DIR})
endif()

# parse addon-bindings.mk to get the list of header files to copy
file(STRINGS ${XBMCROOT}/xbmc/addons/addon-bindings.mk bindings)
string(REPLACE "\n" ";" bindings "${bindings}")
foreach(binding ${bindings})
  string(REPLACE " =" ";" binding "${binding}")
  string(REPLACE "+=" ";" binding "${binding}")
  list(GET binding 1 header)
  # copy the header file to include/xbmc
  file(COPY ${XBMCROOT}/${header} DESTINATION ${XBMC_INCLUDE_DIR})
endforeach()

# generate the proper xbmc-config.cmake file
configure_file(${XBMCROOT}/project/cmake/xbmc-config.cmake.in ${XBMC_LIB_DIR}/xbmc-config.cmake @ONLY)

### get and build all the binary addons
# look for all the addons to be built
file(GLOB addons ${PROJECT_SOURCE_DIR}/addons/*.txt)
foreach(addon ${addons})
  file(STRINGS ${addon} def)
  string(REPLACE " " ";" def ${def})
  list(GET def 0 id)
  list(GET def 1 url)
  list(GET def 2 revision)

  # make sure the output directory is clean
  if(EXISTS "${OUTPUT_DIR}/${id}")
    file(REMOVE_RECURSE "${OUTPUT_DIR}/${id}/")
  endif()

  # add the addon as an external project for automatic building
  externalproject_add(${id}
                      GIT_REPOSITORY ${url}/${id}
                      GIT_TAG ${revision}
                      INSTALL_DIR "${OUTPUT_DIR}"
                      CMAKE_ARGS -DCMAKE_PREFIX_PATH=${prefix}
                                 -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                                 -DCMAKE_BUILD_TYPE=Release
                                 -DPACKAGE_ZIP=1                            #needed for project installing
                                 -DBUILD_SHARED_LIBS=1
                     )
endforeach()
