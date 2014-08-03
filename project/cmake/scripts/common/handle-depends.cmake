# handle addon depends
function(add_addon_depends addon searchpath)
  # input: string searchpath 

  set(OUTPUT_DIR ${DEPENDS_PATH})
  file(GLOB_RECURSE cmake_input_files ${searchpath}/${CORE_SYSTEM_NAME}/*.txt)
  file(GLOB_RECURSE cmake_input_files2 ${searchpath}/common/*.txt)
  list(APPEND cmake_input_files ${cmake_input_files2})
  #message(FATAL_ERROR "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX} - cmake_input_files: ${cmake_input_files}")
  message(STATUS "DEPENDS_PATH: ${DEPENDS_PATH} - cmake_input_files: ${cmake_input_files}")

  foreach(file ${cmake_input_files})
    if(NOT (file MATCHES CMakeLists.txt OR
            file MATCHES install.txt OR
            file MATCHES noinstall.txt OR
            file MATCHES flags.txt OR
            file MATCHES deps.txt))
      message(STATUS "Processing ${file}")
      file(STRINGS ${file} def)
      separate_arguments(def)
      list(LENGTH def deflength)
      get_filename_component(dir ${file} PATH)

    # get the id and url of the dependency
    set(url "")
    if(NOT "${def}" STREQUAL "")
      # read the id and the url from the file
      list(GET def 0 id)
      if(deflength GREATER 1)
        list(GET def 1 url)
        message(STATUS "${id} url: ${url}")
      endif()
    else()
      # read the id from the filename
      get_filename_component(id ${file} NAME_WE)
    endif()

    # check if there are any library specific flags that need to be passed on
    if(EXISTS ${dir}/flags.txt})
      file(STRINGS ${dir}/flags.txt extraflags)
      message(STATUS "${id} extraflags: ${extraflags}")
    endif()

    set(BUILD_ARGS -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
                   -DOUTPUT_DIR=${DEPENDS_PATH}
                   -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                   -DCMAKE_USER_MAKE_RULES_OVERRIDE=${CMAKE_USER_MAKE_RULES_OVERRIDE}
                   -DCMAKE_USER_MAKE_RULES_OVERRIDE_CXX=${CMAKE_USER_MAKE_RULES_OVERRIDE_CXX}
                   -DCMAKE_INSTALL_PREFIX=${DEPENDS_PATH}
                   -DARCH_DEFINES=${ARCH_DEFINES}
                   -DENABLE_STATIC=1
                   -DBUILD_SHARED_LIBS=0
                   "${extraflags}")

    if(CMAKE_TOOLCHAIN_FILE)
      list(APPEND BUILD_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
      MESSAGE("toolchain specified")
      MESSAGE(${BUILD_ARGS})
    endif()

    # if there's a CMakeLists.txt use it to prepare the build
    if(EXISTS ${dir}/CMakeLists.txt)
      file(APPEND ${CMAKE_BINARY_DIR}/build/${id}/tmp/patch.cmake
           "file(COPY ${dir}/CMakeLists.txt
                 DESTINATION ${CMAKE_BINARY_DIR}/build/${id}/src/${id})\n")
      set(PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/build/${id}/tmp/patch.cmake)
    else()
      set(PATCH_COMMAND "")
    endif()

    # check if we have patches to apply
    file(GLOB patches ${dir}/*.patch)
    list(SORT patches)
    foreach(patch ${patches})
      set(PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/build/${id}/tmp/patch.cmake)
      file(APPEND ${CMAKE_BINARY_DIR}/build/${id}/tmp/patch.cmake
           "execute_process(COMMAND patch -p1 -i ${patch})\n")
    endforeach()


    # if there's an install.txt use it to properly install the built files
    if(EXISTS ${dir}/install.txt)
      set(INSTALL_COMMAND INSTALL_COMMAND ${CMAKE_COMMAND}
                                          -DINPUTDIR=${CMAKE_BINARY_DIR}/build/${id}/src/${id}-build/
                                          -DINPUTFILE=${dir}/install.txt
                                          -DDESTDIR=${DEPENDS_PATH}
                                          -DENABLE_STATIC=1
                                          "${extraflags}"
                                          -P ${PROJECT_SOURCE_DIR}/install.cmake)
    elseif(EXISTS ${dir}/noinstall.txt)
      set(INSTALL_COMMAND INSTALL_COMMAND "")
    else()
      set(INSTALL_COMMAND "")
    endif()

    # check if there's a deps.txt containing dependencies on other libraries
    if(EXISTS ${dir}/deps.txt})
      file(STRINGS ${dir}/deps.txt deps)
      message(STATUS "${id} dependencies: ${deps}")
      set(DEPENDS_COMMAND DEPENDS ${deps})
    else()
      set(DEPENDS_COMMAND "")
    endif()

    # prepare the setup of the call to externalproject_add()
    set(EXTERNALPROJECT_SETUP PREFIX build/${id}
                              CMAKE_ARGS ${BUILD_ARGS}
                              PATCH_COMMAND ${PATCH_COMMAND}
                              "${INSTALL_COMMAND}"
                              "${DEPENDS_COMMAND}")

    # if there's an url defined we need to pass that to externalproject_add()
    if(DEFINED url AND NOT "${url}" STREQUAL "")
      # check if there's a third parameter in the file
      if(deflength GREATER 2)
        # the third parameter is considered as a revision of a git repository
        list(GET def 2 revision)
        externalproject_add(${id}
                            GIT_REPOSITORY ${url}
                            GIT_TAG ${revision}
                            "${EXTERNALPROJECT_SETUP}"
                           )
      else()
        externalproject_add(${id}
                            URL ${url}
                            CONFIGURE_COMMAND PKG_CONFIG_PATH=${OUTPUT_DIR}/lib/pkgconfig 
                                            LD_RUN_PATH=${OUTPUT_DIR}/lib
                                            LDFLAGS="-L${OUTPUT_DIR}/lib"
                                            CFLAGS="-L${OUTPUT_DIR}/lib"
                                            CPPFLAGS="-L${OUTPUT_DIR}/lib"
                                            ${CMAKE_COMMAND}
                                            ${CMAKE_BINARY_DIR}/build/${id}/src/${id}
                                            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                            -DOUTPUT_DIR=${OUTPUT_DIR}
                                            -DCMAKE_PREFIX_PATH=${OUTPUT_DIR}
                                            -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}
                                            -DCMAKE_EXE_LINKER_FLAGS=-L${OUTPUT_DIR}/lib
                            "${EXTERNALPROJECT_SETUP}"
                           )
      endif()
    else()
      externalproject_add(${id}
                          SOURCE_DIR ${dir}
                          "${EXTERNALPROJECT_SETUP}"
                         )
    endif()
   add_dependencies(${addon} ${id})
   endif()
  endforeach()
endfunction()

