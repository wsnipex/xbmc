# Track if at least one dependency has been found. Everything else is likely an
# error either in DEPENDENCYS_TO_BUILD or in the directory configuration.
set(SUPPORTED_DEPENDENCY_FOUND FALSE)

foreach(dependency ${dependencys})
  if(NOT (dependency MATCHES platforms.txt))
    file(STRINGS ${dependency} def)
    string(REPLACE " " ";" def ${def})
    list(GET def 0 id)

    set(DEPENDENCY_FOUND FALSE)
    # try to find a perfect match
    list(FIND DEPENDENCYS_TO_BUILD ${id} idx)
    if(idx GREATER -1 OR "${DEPENDENCYS_TO_BUILD}" STREQUAL "all")
      set(DEPENDENCY_FOUND TRUE)
    # Maybe we have a regex
    elseif(id MATCHES "${DEPENDENCYS_TO_BUILD}")
      message(STATUS "Pattern ${DEPENDENCYS_TO_BUILD} matches ${id}, building dependency")
      set(DEPENDENCY_FOUND TRUE)
    endif()

    if(DEPENDENCY_FOUND)
      message(STATUS "\n-- ---- Configuring dependency ${dependency} ----")
      set(SUPPORTED_DEPENDENCY_FOUND TRUE)

      get_filename_component(dir ${dependency} DIRECTORY)

      # check if the dependency has a platforms.txt
      set(platform_found FALSE)
      check_target_platform(${dir} ${CORE_SYSTEM_NAME} platform_found)

      if(${platform_found})
        # make sure the output directory is clean
        file(REMOVE_RECURSE "${CMAKE_INSTALL_PREFIX}/${id}/")

        # get the URL and revision of the dependency
        list(LENGTH def deflength)
        list(GET def 1 url)

        set(archive_name ${id})
        if(DEPENDENCY_SRC_PREFIX)
          set(SOURCE_DIR ${DEPENDENCY_SRC_PREFIX}/${id})
          set(archive_name "")
        else()
          set(SOURCE_DIR "")
        endif()

        # if there is a 3rd parameter in the file, we consider it a git revision
        if(deflength GREATER 2 AND "${SOURCE_DIR}" STREQUAL "")
          list(GET def 2 revision)

          # we need access to a git executable
          find_package(Git REQUIRED)

          # resolve revision to git hash
          execute_process(COMMAND ${GIT_EXECUTABLE} ls-remote ${url} ${revision} OUTPUT_VARIABLE revision_hash)
          # git ls-remote only works on branches and tag names but not on revisions
          if(NOT "${revision_hash}" STREQUAL "")
            string(REPLACE "\t" ";" revision_list ${revision_hash})
            list(GET revision_list 0 revision_hash)
            message(STATUS "${id}: git branch/tag ${revision} resolved to hash: ${revision_hash}")
            set(revision ${revision_hash})
          endif()

          # Note: downloading specific revisions via http in the format below is probably github specific
          # if we ever use other repositories, this might need adapting
          set(url ${url}/archive/${revision}.tar.gz)
          set(archive_name ${archive_name}-${revision})
        elseif("${SOURCE_DIR}" STREQUAL "")
          # check if the URL starts with file://
          string(REGEX MATCH "^file://.*$" local_url "${url}")

          #if not we assume this to be a local directory
          if(local_url)
            # this is not an archive
            set(archive_name "")

            # remove the file:// protocol from the URL
            string(REPLACE "file://" "" SOURCE_DIR "${url}")

            # on win32 we may have to remove another leading /
            if(WIN32)
              # check if the path is a local path
              string(REGEX MATCH "^/.*$" local_path "${SOURCE_DIR}")
              if(local_path)
                string(SUBSTRING "${SOURCE_DIR}" 1 -1 SOURCE_DIR)
              endif()
            endif()
          endif()
        endif()

        # download the dependency if necessary
        if(NOT "${archive_name}" STREQUAL "")
          # download and extract the dependency
          if(NOT DEPENDENCY_TARBALL_CACHING OR NOT EXISTS ${BUILD_DIR}/download/${archive_name}.tar.gz)
            # cleanup any of the previously downloaded archives of this dependency
            file(GLOB archives "${BUILD_DIR}/download/${id}*.tar.gz")
            if(archives)
              message(STATUS "Removing old archives of ${id}: ${archives}")
              file(REMOVE ${archives})
            endif()

            # download the dependency
            file(DOWNLOAD "${url}" "${BUILD_DIR}/download/${archive_name}.tar.gz" STATUS dlstatus LOG dllog SHOW_PROGRESS)
            list(GET dlstatus 0 retcode)
            if(NOT ${retcode} EQUAL 0)
              file(REMOVE ${BUILD_DIR}/download/${archive_name}.tar.gz)
              message(STATUS "ERROR downloading ${url} - status: ${dlstatus} log: ${dllog}")
              # add a dummy target for dependencys to get it in dependencys failure file
              list(APPEND ALL_DEPENDENCYS_BUILDING ${id})
              add_custom_target(${id} COMMAND ${CMAKE_COMMAND} -E echo "IGNORED ${id} - download failed" COMMAND exit 1)
              continue()
            endif()
          endif()

          # remove any previously extracted version of the dependency
          file(REMOVE_RECURSE "${BUILD_DIR}/${id}")

          # extract the dependency from the archive
          execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzvf ${BUILD_DIR}/download/${archive_name}.tar.gz
                          WORKING_DIRECTORY ${BUILD_DIR})
          file(GLOB extract_dir "${BUILD_DIR}/${archive_name}*")
          if(extract_dir STREQUAL "")
            message(FATAL_ERROR "${id}: error extracting ${BUILD_DIR}/download/${archive_name}.tar.gz")
          else()
            file(RENAME "${extract_dir}" "${BUILD_DIR}/${id}")
          endif()

          set(SOURCE_DIR ${BUILD_DIR}/${id})
        endif()

        if(NOT "${SOURCE_DIR}" STREQUAL "" AND EXISTS ${SOURCE_DIR})
          # create a list of dependencys we are building
          list(APPEND ALL_DEPENDENCYS_BUILDING ${id})

          # setup the buildsystem for the dependency
          externalproject_add(${id}
                              SOURCE_DIR ${SOURCE_DIR}
                              INSTALL_DIR ${DEPENDENCY_INSTALL_DIR}
                              CMAKE_ARGS ${BUILD_ARGS})

          # add a custom step to the external project between the configure and the build step which will always
          # be executed and therefore forces a re-build of all changed files
          externalproject_add_step(${id} forcebuild
                                   COMMAND ${CMAKE_COMMAND} -E echo "Force build of ${id}"
                                   DEPENDEES configure
                                   DEPENDERS build
                                   ALWAYS 1)

          # add "kodi-platform" as a dependency to every dependency
          add_dependencies(${id} kodi-platform)

          set(${id}_DEPENDS_DIR ${SOURCE_DIR}/depends)

          if(EXISTS ${${id}_DEPENDS_DIR})
            include(${CORE_SOURCE_DIR}/cmake/scripts/common/HandleDepends.cmake)
            add_dependency_depends(${id} ${${id}_DEPENDS_DIR})
            if(${id}_DEPS AND NOT "${${id}_DEPS}" STREQUAL "")
              message(STATUS "${id} DEPENDENCIES: ${${id}_DEPS}")
              add_dependencies(${id} ${${id}_DEPS})
            endif()
          endif()

          if(CROSS_AUTOCONF AND AUTOCONF_FILES)
            if(EXISTS ${SOURCE_DIR}/bootstrap/autoreconf.txt)
              file(STRINGS ${SOURCE_DIR}/bootstrap/autoreconf.txt conf_dirs)
              foreach(conf_dir ${conf_dirs})
                foreach(afile ${AUTOCONF_FILES})
                  message(STATUS "copying ${afile} to ${SOURCE_DIR}/${conf_dir}")
                  file(COPY ${afile} DESTINATION ${SOURCE_DIR}/${conf_dir})
                endforeach()
              endforeach()
            endif()
          endif()

          # create a forwarding target to the dependency-package target
          add_custom_target(package-${id}
                    COMMAND ${CMAKE_COMMAND} --build ${id}-prefix/src/${id}-build --target dependency-package
                    DEPENDS ${id})
          add_dependencies(package-dependencys package-${id})

        else()
          message(FATAL_ERROR "${id}: invalid or missing dependency source directory at ${SOURCE_DIR}")
        endif()
      else()
        # add a dummy target for dependencys that are unsupported on this platform
        add_custom_target(${id} COMMAND ${CMAKE_COMMAND} -E echo "IGNORED ${id} - not supported on ${CORE_SYSTEM_NAME}\n")
      endif()
    endif()
  endif()
endforeach()
