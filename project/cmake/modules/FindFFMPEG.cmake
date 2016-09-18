# external FFMPEG
if(FFMPEG_PATH)
  set(ENV{PKG_CONFIG_PATH} "${FFMPEG_PATH}/lib/pkgconfig")
  list(APPEND CMAKE_PREFIX_PATH ${FFMPEG_PATH})
  set(ENABLE_INTERNAL_FFMPEG OFF)
endif()

# FFMPEG v3.1
set(REQUIRED_FFMPEG_VERSION 3.1)
set(FFMPEG_PKGS libavcodec>=57.48.101
                libavfilter>=6.47.100
                libavformat>=57.41.100
                libavutil>=55.28.100
                libswscale>=4.1.100
                libswresample>=2.1.100
                libpostproc>=54.0.100)

if(PKG_CONFIG_FOUND AND NOT WIN32)
  pkg_check_modules(PC_FFMPEG ${FFMPEG_PKGS} QUIET)
  string(REGEX REPLACE "framework;" "framework " PC_FFMPEG_LDFLAGS "${PC_FFMPEG_LDFLAGS}")
endif()

find_path(FFMPEG_INCLUDE_DIRS libavcodec/avcodec.h libavfilter/avfilter.h libavformat/avformat.h
                              libavutil/avutil.h libswscale/swscale.h libpostproc/postprocess.h
          PATH_SUFFIXES ffmpeg
          PATHS ${PC_FFMPEG_INCLUDE_DIRS}
          NO_DEFAULT_PATH)
find_path(FFMPEG_INCLUDE_DIRS libavcodec/avcodec.h libavfilter/avfilter.h libavformat/avformat.h
                              libavutil/avutil.h libswscale/swscale.h libpostproc/postprocess.h)

find_library(FFMPEG_LIBAVCODEC
             NAMES avcodec libavcodec
             PATH_SUFFIXES ffmpeg/libavcodec
             PATHS ${PC_FFMPEG_libavcodec_LIBDIR}
             NO_DEFAULT_PATH)
find_library(FFMPEG_LIBAVCODEC NAMES avcodec libavcodec PATH_SUFFIXES ffmpeg/libavcodec)

find_library(FFMPEG_LIBAVFILTER
             NAMES avfilter libavfilter
             PATH_SUFFIXES ffmpeg/libavfilter
             PATHS ${PC_FFMPEG_libavfilter_LIBDIR}
             NO_DEFAULT_PATH)
find_library(FFMPEG_LIBAVFILTER NAMES avfilter libavfilter PATH_SUFFIXES ffmpeg/libavfilter)

find_library(FFMPEG_LIBAVFORMAT
             NAMES avformat libavformat
             PATH_SUFFIXES ffmpeg/libavformat
             PATHS ${PC_FFMPEG_libavformat_LIBDIR}
             NO_DEFAULT_PATH)
find_library(FFMPEG_LIBAVFORMAT NAMES avformat libavformat PATH_SUFFIXES ffmpeg/libavformat)

find_library(FFMPEG_LIBAVUTIL
             NAMES avutil libavutil
             PATH_SUFFIXES ffmpeg/libavutil
             PATHS ${PC_FFMPEG_libavutil_LIBDIR}
             NO_DEFAULT_PATH)
find_library(FFMPEG_LIBAVUTIL NAMES avutil libavutil PATH_SUFFIXES ffmpeg/libavutil)

find_library(FFMPEG_LIBSWSCALE
             NAMES swscale libswscale
             PATH_SUFFIXES ffmpeg/libswscale
             PATHS ${PC_FFMPEG_libswscale_LIBDIR}
             NO_DEFAULT_PATH)
find_library(FFMPEG_LIBSWSCALE NAMES swscale libswscale PATH_SUFFIXES ffmpeg/libswscale)

find_library(FFMPEG_LIBSWRESAMPLE
             NAMES swresample libswresample
             PATH_SUFFIXES ffmpeg/libswresample
             PATHS ${PC_FFMPEG_libswresample_LIBDIR}
             NO_DEFAULT_PATH)
find_library(FFMPEG_LIBSWRESAMPLE NAMES NAMES swresample libswresample PATH_SUFFIXES ffmpeg/libswresample)

find_library(FFMPEG_LIBPOSTPROC
             NAMES postproc libpostproc
             PATH_SUFFIXES ffmpeg/libpostproc
             PATHS ${PC_FFMPEG_libpostproc_LIBDIR}
             NO_DEFAULT_PATH)
find_library(FFMPEG_LIBPOSTPROC NAMES postproc libpostproc PATH_SUFFIXES ffmpeg/libpostproc)

if(PC_FFMPEG_FOUND
   AND PC_FFMPEG_libavcodec_VERSION
   AND PC_FFMPEG_libavfilter_VERSION
   AND PC_FFMPEG_libavformat_VERSION
   AND PC_FFMPEG_libavutil_VERSION
   AND PC_FFMPEG_libswscale_VERSION
   AND PC_FFMPEG_libswresample_VERSION
   AND PC_FFMPEG_libpostproc_VERSION)
  set(FFMPEG_VERSION ${REQUIRED_FFMPEG_VERSION})


  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(FFMPEG
                                    VERSION_VAR FFMPEG_VERSION
                                    REQUIRED_VARS FFMPEG_INCLUDE_DIRS
                                                  FFMPEG_LIBAVCODEC
                                                  FFMPEG_LIBAVFILTER
                                                  FFMPEG_LIBAVFORMAT
                                                  FFMPEG_LIBAVUTIL
                                                  FFMPEG_LIBSWSCALE
                                                  FFMPEG_LIBSWRESAMPLE
                                                  FFMPEG_LIBPOSTPROC
                                                  FFMPEG_VERSION
                                    FAIL_MESSAGE "FFmpeg ${REQUIRED_FFMPEG_VERSION} not found, please consider using -DENABLE_INTERNAL_FFMPEG=ON")

else()
  message(WARNING "FFmpeg ${REQUIRED_FFMPEG_VERSION} not found, falling back to internal build")
  #set(BUILD_INTERNAL_FFMPEG ON)
  set(ENABLE_INTERNAL_FFMPEG ON)
  set(ENABLE_INTERNAL_FFMPEG ON CACHE BOOL "enable internal ffmpeg" FORCE)
  unset(FFMPEG_INCLUDE_DIRS)
  unset(FFMPEG_INCLUDE_DIRS CACHE)
  unset(FFMPEG_LIBRARIES)
  unset(FFMPEG_LIBRARIES CACHE)
  unset(FFMPEG_DEFINITIONS)
  unset(FFMPEG_DEFINITIONS CACHE)
endif()

if(FFMPEG_FOUND)
  set(FFMPEG_LIBRARIES ${FFMPEG_LIBAVCODEC} ${FFMPEG_LIBAVFILTER}
                       ${FFMPEG_LIBAVFORMAT} ${FFMPEG_LIBAVUTIL}
                       ${FFMPEG_LIBSWSCALE} ${FFMPEG_LIBSWRESAMPLE}
                       ${FFMPEG_LIBPOSTPROC})
  set(FFMPEG_LDFLAGS ${PC_FFMPEG_LDFLAGS} CACHE STRING "ffmpeg linker flags")

  # check if ffmpeg libs are statically linked
  set(FFMPEG_LIB_TYPE SHARED)
  foreach(_fflib IN LISTS FFMPEG_LIBRARIES)
    if(${_fflib} MATCHES ".+\.a$" AND PC_FFMPEG_STATIC_LDFLAGS)
      set(FFMPEG_LDFLAGS ${PC_FFMPEG_STATIC_LDFLAGS} CACHE STRING "ffmpeg linker flags" FORCE)
      set(FFMPEG_LIB_TYPE STATIC)
      break()
    endif()
  endforeach()

  list(APPEND FFMPEG_DEFINITIONS -DFFMPEG_VER_SHA=\"${FFMPEG_VERSION}\")

  if(NOT TARGET ffmpeg)
    add_library(ffmpeg ${FFMPEG_LIB_TYPE} IMPORTED)
    set_target_properties(ffmpeg PROPERTIES
                                 FOLDER "External Projects"
                                 IMPORTED_LOCATION "${FFMPEG_LIBRARIES}"
                                 INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}"
                                 INTERFACE_LINK_LIBRARIES "${FFMPEG_LDFLAGS}"
                                 INTERFACE_COMPILE_DEFINITIONS "${FFMPEG_DEFINITIONS}")
  endif()
endif()

# Internal FFMPEG
if(ENABLE_INTERNAL_FFMPEG)# AND BUILD_INTERNAL_FFMPEG)
  include(ExternalProject)
  file(STRINGS ${CORE_SOURCE_DIR}/tools/depends/target/ffmpeg/FFMPEG-VERSION VER)
  string(REGEX MATCH "VERSION=[^ ]*$.*" FFMPEG_VER "${VER}")
  list(GET FFMPEG_VER 0 FFMPEG_VER)
  string(SUBSTRING "${FFMPEG_VER}" 8 -1 FFMPEG_VER)
  string(REGEX MATCH "BASE_URL=([^ ]*)" FFMPEG_BASE_URL "${VER}")
  list(GET FFMPEG_BASE_URL 0 FFMPEG_BASE_URL)
  string(SUBSTRING "${FFMPEG_BASE_URL}" 9 -1 FFMPEG_BASE_URL)

  # allow user to override the download URL with a local tarball
  # needed for offline build envs
  if(FFMPEG_URL)
    get_filename_component(FFMPEG_URL "${FFMPEG_URL}" ABSOLUTE)
  else()
    set(FFMPEG_URL ${FFMPEG_BASE_URL}/${FFMPEG_VER}.tar.gz)
  endif()
  if(VERBOSE)
    message(STATUS "FFMPEG_URL: ${FFMPEG_URL}")
  endif()

  if(CMAKE_CROSSCOMPILING)
    set(CROSS_ARGS -DDEPENDS_PATH=${DEPENDS_PATH}
                   -DPKG_CONFIG_EXECUTABLE=${PKG_CONFIG_EXECUTABLE}
                   -DCROSSCOMPILING=${CMAKE_CROSSCOMPILING}
                   -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                   -DCORE_SYSTEM_NAME=${CORE_SYSTEM_NAME}
                   -DCPU=${WITH_CPU}
                   -DOS=${OS}
                   -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                   -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                   -DCMAKE_AR=${CMAKE_AR}
                   -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
                   -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS})
  endif()

  
  set(FFMPEG_ARGS "-DFFMPEG_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
                  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                  -DFFMPEG_VER=${FFMPEG_VER}
                  -DCORE_SYSTEM_NAME=${CORE_SYSTEM_NAME}
                  ${CROSS_ARGS}")
  set(FFMPEG_PATCH_CMD "${CMAKE_COMMAND} -E copy
                       ${CORE_SOURCE_DIR}/tools/depends/target/ffmpeg/CMakeLists.txt
                       <SOURCE_DIR> &&
                       ${CMAKE_COMMAND} -E copy
                       ${CORE_SOURCE_DIR}/tools/depends/target/ffmpeg/FindGnuTls.cmake
                       <SOURCE_DIR>")

  build_external_project(ffmpeg "${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/depends" "${FFMPEG_URL}" "${FFMPEG_PATCH_CMD}" "${FFMPEG_ARGS}")
  unset(__pkg_config_checked_PC_FFMPEG)
  unset(__pkg_config_checked_PC_FFMPEG CACHE)
  unset(FFMPEG_LIBAVCODEC CACHE)
  unset(FFMPEG_LIBAVFILTER CACHE)
  unset(FFMPEG_LIBAVFORMAT CACHE)
  unset(FFMPEG_LIBAVUTIL CACHE)
  unset(FFMPEG_LIBPOSTPROC CACHE)
  unset(FFMPEG_LIBSWRESAMPLE CACHE)
  unset(FFMPEG_LIBSWSCALE CACHE)

  set(ENABLE_INTERNAL_FFMPEG OFF)
  set(ENABLE_INTERNAL_FFMPEG OFF CACHE BOOL "enable internal ffmpeg" FORCE)
  #set(BUILD_INTERNAL_FFMPEG OFF)
  #set(BUILD_INTERNAL_FFMPEG OFF CACHE BOOL "enable internal ffmpeg" FORCE)
  set(FFMPEG_PATH "${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}")
  find_package(FFMPEG REQUIRED)
endif()

mark_as_advanced(FFMPEG_INCLUDE_DIRS FFMPEG_LIBRARIES FFMPEG_LDFLAGS FFMPEG_DEFINITIONS FFMPEG_FOUND)
