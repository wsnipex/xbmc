cmake_minimum_required(VERSION 3.4)
project(kodi-depends LANGUAGES CXX C ASM)

include(ExternalProject)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(DEPENDS_SOURCE_DIR ${PROJECT_SOURCE_DIR})

if(NOT CORE_SOURCE_DIR)
  set(CORE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../..)
else()
  file(TO_CMAKE_PATH "${CORE_SOURCE_DIR}" CORE_SOURCE_DIR)
endif()
get_filename_component(CORE_SOURCE_DIR "${CORE_SOURCE_DIR}" ABSOLUTE)

if(NOT BUILD_DIR)
  set(BUILD_DIR "${CMAKE_BINARY_DIR}/build")
else()
  file(TO_CMAKE_PATH "${BUILD_DIR}" BUILD_DIR)
endif()
get_filename_component(BUILD_DIR "${BUILD_DIR}" ABSOLUTE)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR} ${CORE_SOURCE_DIR}/cmake/modules)

# general
option(VERBOSE      "Enable verbose output?" OFF)
set(KODI_MIRROR     "http://mirrors.kodi.tv" CACHE STRING "Mirror server")
set(TARBALL_DIR "${BUILD_DIR}/download" CACHE STRING "Tarball download dir")
option(DEPENDENCY_TARBALL_CACHING "Cache downloaded files" ON)
set(TARGET_PLATFORM ${TARGET_PLATFORM} CACHE STRING "Target device")
#option(WITH_ARCH      "build with given arch" AUTO)
#option(WITH_CPU       "build with given cpu" OFF)
option(ENABLE_CCACHE   "Enable Ccache support" ON)

# android
if(CORE_SYSTEM_NAME STREQUAL android)
  set(ANDROID_API "21" CACHE VALUE "Android API level")
  set_property(CACHE ANDROID_API PROPERTY TYPE STRING)
  set(ANDROID_NDK "${ANDROID_NDK}" CACHE PATH "Path to Android NDK")
  #set_property(CACHE ANDROID_NDK PROPERTY TYPE PATH) 
  set(ANDROID_SDK "${ANDROID_SDK}" CACHE PATH "Path to Android SDK")
  #set_property(CACHE ANDROID_SDK PROPERTY TYPE PATH)
endif()


if(NOT TARGET_PLATFORM)
  message(FATAL_ERROR "TARGET_PLATFORM not set")
endif()

if(EXISTS ${DEPENDS_SOURCE_DIR}/target_devices/${TARGET_PLATFORM})
  include(${DEPENDS_SOURCE_DIR}/target_devices/${TARGET_PLATFORM})
else()
  message(FATAL_ERROR "${DEPENDS_SOURCE_DIR}/target_devices/${TARGET_PLATFORM} not found")
endif()

list(APPEND final_message "BUILD_DIR: ${BUILD_DIR}")
list(APPEND final_message "NATIVE_C_COMPILER: ${CMAKE_C_COMPILER}")
list(APPEND final_message "NATIVE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")

set(APP_PREFIX "${CMAKE_INSTALL_PREFIX}")


set(BUILD_ARGS -DBASE_URL=${KODI_MIRROR}/build-deps/sources
               -DTARBALL_DIR=${TARBALL_DIR}
               -DDEPENDENCY_TARBALL_CACHING=${DEPENDENCY_TARBALL_CACHING}
               -DCORE_SOURCE_DIR=${CORE_SOURCE_DIR}
               -DBUILD_DIR=${BUILD_DIR}
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DCMAKE_USER_MAKE_RULES_OVERRIDE=${CMAKE_USER_MAKE_RULES_OVERRIDE}
               -DCMAKE_USER_MAKE_RULES_OVERRIDE_CXX=${CMAKE_USER_MAKE_RULES_OVERRIDE_CXX}
               -DENABLE_CCACHE=${ENABLE_CCACHE}
               ${BUILD_ARGS_ext}
               ${BUILD_ARGS_device})

set(BUILD_ARGS_native
               -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
               -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}/native-${CMAKE_BUILD_TYPE}
               -DPACKAGE_CONFIG_PATH=${CMAKE_INSTALL_PREFIX}/native-${CMAKE_BUILD_TYPE}/lib/pkgconfig)

set(BUILD_ARGS_target
               -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME_TARGET}
               -DCORE_SYSTEM_NAME=${CORE_SYSTEM_NAME}
               -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
               -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}/${TARGET_PLATFORM}-${CMAKE_BUILD_TYPE}
               -DPACKAGE_CONFIG_PATH=${CMAKE_INSTALL_PREFIX}/${TARGET_PLATFORM}-${CMAKE_BUILD_TYPE}/lib/pkgconfig
               -DCMAKE_C_FLAGS=${TARGET_C_FLAGS}
               -DCMAKE_CXX_FLAGS=${TARGET_CXX_FLAGS})


externalproject_add(native_depends
               SOURCE_DIR ${PROJECT_SOURCE_DIR}/native
               INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/native-${CMAKE_BUILD_TYPE}
               INSTALL_COMMAND cmake -E echo "Skipping install step."
               CMAKE_ARGS ${BUILD_ARGS_native} ${BUILD_ARGS})



#externalproject_add(target_depends
#               SOURCE_DIR ${PROJECT_SOURCE_DIR}/target
#               INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${TARGET_PLATFORM}-${CMAKE_BUILD_TYPE}
#               CMAKE_ARGS ${BUILD_ARGS_target} ${BUILD_ARGS})


list(APPEND final_message "CMAKE_CXX_COMPILER_TARGET: ${CMAKE_CXX_COMPILER_TARGET}")


# Status
message(STATUS "#---- CONFIGURATION ----#")
foreach(msg ${final_message})
  message(STATUS ${msg})
endforeach()

if(VERBOSE)
  message(STATUS "\n#--------------- Internal Variables -------------#")
  message(STATUS "DEPLIBS: ${DEPLIBS}")
  message(STATUS "core_DEPENDS: ${core_DEPENDS}")
  message(STATUS "#---------------------------------------------#")
  message(STATUS "SYSTEM_LDFLAGS        : ${SYSTEM_LDFLAGS}")
  message(STATUS "CMAKE_DL_LIBS         : ${CMAKE_DL_LIBS}")
  message(STATUS "CMAKE_C_FLAGS         : ${CMAKE_C_FLAGS}")
  message(STATUS "CMAKE_CXX_FLAGS       : ${CMAKE_CXX_FLAGS}")
  message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
  message(STATUS "LTO_OPTIMIZATION:     : ${CMAKE_INTERPROCEDURAL_OPTIMIZATION}")
  message(STATUS "#---------------------------------------------#")
  message(STATUS "bindir     : ${bindir}")
  message(STATUS "includedir : ${includedir}")
  message(STATUS "libdir     : ${libdir}")
  message(STATUS "datarootdir: ${datarootdir}")
  message(STATUS "#---------------------------------------------#")
  message(STATUS "BINARY: ${APP_NAME_LC}${APP_BINARY_SUFFIX}")
  message(STATUS "#---------------------------------------------#")
  message(STATUS "GIT_REV: ${APP_SCMID}")
  message(STATUS "#---------------------------------------------#")
  message(STATUS "CPACK_GENERATOR       : ${CPACK_GENERATOR}")
  message(STATUS "CPACK_SOURCE_GENERATOR: ${CPACK_SOURCE_GENERATOR}")
  message(STATUS "#---------------------------------------------#")
endif()
