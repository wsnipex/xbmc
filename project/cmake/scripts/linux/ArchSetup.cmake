set(ARCH_DEFINES -D_LINUX -DTARGET_POSIX -DTARGET_LINUX)
set(SYSTEM_DEFINES -D__STDC_CONSTANT_MACROS -D_FILE_DEFINED
                   -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)
set(PLATFORM_DIR linux)
set(CMAKE_SYSTEM_NAME Linux)
if(WITH_ARCH)
  set(ARCH ${WITH_ARCH})
else()
  if(CPU STREQUAL x86_64)
    set(ARCH x86_64-linux)
  elseif(CPU MATCHES "i.86")
    set(ARCH i486-linux)
  else()
    message(SEND_ERROR "Unknown CPU: ${CPU}")
  endif()
endif()

find_package(CXX11 REQUIRED)
include(LDGOLD)

if(ENABLE_LIRC)
  set(LIRC_DEVICE "\"/dev/lircd\"" CACHE STRING "LIRC device to use")
  set(DEP_DEFINES -DLIRC_DEVICE=${LIRC_DEVICE} -DHAVE_LIRC=1)
endif()

# Code Coverage
if(CMAKE_BUILD_TYPE STREQUAL Coverage)
  set(COVERAGE_TEST_BINARY ${APP_NAME_LC}-test)
  set(COVERAGE_SOURCE_DIR ${CORE_SOURCE_DIR})
  set(COVERAGE_EXCLUDES */test/* lib/* */lib/*)
endif()
