include(${CORE_SOURCE_DIR}/cmake/scripts/common/Macros.cmake)

core_find_versions()

# configure_file without dependency tracking
# configure_file would register additional file dependencies that interfere
# with the ones from add_custom_command (and the generation would happen twice)
function(generate_versioned_file _SRC _DEST)
  file(READ ${CORE_SOURCE_DIR}/${_SRC} file_content)
  string(CONFIGURE "${file_content}" file_content @ONLY)
  file(WRITE ${CMAKE_BINARY_DIR}/${_DEST} "${file_content}")
endfunction()

# add-on xml's
file(GLOB ADDON_XML_IN_FILE ${CORE_SOURCE_DIR}/addons/*/addon.xml.in)

# remove 'xbmc.json', will be created from 'xbmc/interfaces/json-rpc/schema/CMakeLists.txt'
list(REMOVE_ITEM ADDON_XML_IN_FILE ${CORE_SOURCE_DIR}/addons/xbmc.json/addon.xml.in)

foreach(loop_var ${ADDON_XML_IN_FILE})
  list(GET loop_var 0 xml_name)

  string(REPLACE "/addon.xml.in" "" source_dir ${xml_name})
  string(REPLACE ${CORE_SOURCE_DIR} ${CMAKE_BINARY_DIR} dest_dir ${source_dir})
  file(MAKE_DIRECTORY ${dest_dir})

  # copy everything except addon.xml.in to build folder
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${source_dir}" "${CMAKE_BINARY_DIR}/addons"
                  RESULT_VARIABLE copy_result
                  OUTPUT_VARIABLE copy_output
                  ERROR_VARIABLE copy_output)
  if(NOT ${copy_result} EQUAL 0 )
    message(WARN " ---------- GenerateVersionedFiles.cmake: Error ${copy_result} while copying ${source_dir}: ${copy_ouput}, retrying..")
    execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 2)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${source_dir}" "${CMAKE_BINARY_DIR}/addons"
                    RESULT_VARIABLE copy_result
                    OUTPUT_VARIABLE copy_output
                    ERROR_VARIABLE copy_output)
    if(NOT ${copy_result} EQUAL 0 )
      message(FATAL_ERROR " ---------- GenerateVersionedFiles.cmake: Error ${copy_result} while copying ${source_dir}: ${copy_ouput}")
    endif()
  endif()

  configure_file(${source_dir}/addon.xml.in ${dest_dir}/addon.xml @ONLY)

  unset(source_dir)
  unset(dest_dir)
  unset(xml_name)
  unset(copy_result)
  unset(copy_output)
endforeach()


generate_versioned_file(xbmc/CompileInfo.cpp.in ${CORE_BUILD_DIR}/xbmc/CompileInfo.cpp)
