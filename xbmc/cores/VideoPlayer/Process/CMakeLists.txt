file(GLOB OVERRIDES_CPP overrides/${CORE_SYSTEM_NAME}/*.cpp)
file(GLOB OVERRIDES_H overrides/${CORE_SYSTEM_NAME}/*.h)

if(NOT OVERRIDES_CPP STREQUAL "")
  add_definitions(-DPLATFORM_OVERRIDE)
endif()

set(SOURCES ProcessInfo.cpp ${OVERRIDES_CPP})
set(HEADERS ProcessInfo.h ${OVERRIDES_H})

core_add_library(process)
