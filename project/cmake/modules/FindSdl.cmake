# - Try to find SDL
# Once done this will define
#
# SDL_FOUND - system has libSDL
# SDL_INCLUDE_DIRS - the libSDL include directory
# SDL_LIBRARIES - The libSDL libraries

if(PKG_CONFIG_FOUND AND NOT SDLIMAGE_INCLUDE_DIR)
  pkg_check_modules (SDL sdl)
elseif(NOT SDLIMAGE_INCLUDE_DIR)
  find_package(SDL)
  list(APPEND SDL_INCLUDE_DIRS ${SDL_INCLUDE_DIR})
  list(APPEND SDL_LIBRARIES ${SDL_LIBRARIES})
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Sdl DEFAULT_MSG SDL_INCLUDE_DIRS SDL_LIBRARIES)

  list(APPEND SDL_DEFINITIONS -DHAVE_SDL=1)

endif()


mark_as_advanced(SDL_INCLUDE_DIRS SDL_LIBRARIES SDL_DEFINITIONS)
