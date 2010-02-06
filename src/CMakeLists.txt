configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/build_info.cc.cmake ${CMAKE_CURRENT_BINARY_DIR}/build_info.cc)

file(GLOB_RECURSE WL_SRCS_ALL *.cc)

#file (GLOB WL_SRCS *.cc economy/*.cc trigger/*.cc scripting/*.cc)
#create WL_SRCS and WL_TEST_SRCS
#convention: any directory called "test" contains test sources, including subdirectories
set(WL_SRCS "")
set(WL_TEST_SRCS "")
foreach (sourcefile ${WL_SRCS_ALL})
  if (${sourcefile} MATCHES ".*/test/.*")
    list (APPEND WL_TEST_SRCS ${sourcefile})
  else (${sourcefile} MATCHES ".*/test/.*")
    list (APPEND WL_SRCS ${sourcefile})
  endif (${sourcefile} MATCHES ".*/test/.*")
endforeach (sourcefile ${WL_SRCS_ALL})

execute_process (COMMAND cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/codecheck-stamps)
set_directory_properties (PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/codecheck-stamps/*_codecheck_stamp_)

foreach(sourcefile ${WL_SRCS} ${WL_TEST_SRCS})
  string(REGEX REPLACE "\\.|\\/" "_" sourcenoslash ${sourcefile})
#  get_filename_component(sourcename ${sourcefile} NAME) 
  set(stamp ${sourcenoslash}_codecheck_stamp_)
  set(PREFIX_WL_SRCS ${PREFIX_WL_SRCS};${CMAKE_CURRENT_BINARY_DIR}/codecheck-stamps/${stamp})
  if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/codecheck-stamps/${stamp} OR ${sourcefile} IS_NEWER_THAN ${CMAKE_CURRENT_BINARY_DIR}/codecheck-stamps/${stamp})
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/codecheck-stamps/${stamp}
      COMMAND "${WL_SOURCE_CHECKER}" -c "${sourcefile}"
      COMMAND cmake -E touch ${CMAKE_CURRENT_BINARY_DIR}/codecheck-stamps/${stamp}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#      DEPENDS ${sourcefile}
    )
  endif (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/codecheck-stamps/${stamp} OR ${sourcefile} IS_NEWER_THAN ${CMAKE_CURRENT_BINARY_DIR}/codecheck-stamps/${stamp})
endforeach(sourcefile ${WL_SRCS} ${WL_TEST_SRCS})

add_custom_target(codecheck
  DEPENDS ${PREFIX_WL_SRCS}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

add_subdirectory(ai)
add_subdirectory(economy)
add_subdirectory(editor)
add_subdirectory(events)
add_subdirectory(game_io)
add_subdirectory(graphic)
add_subdirectory(io)
add_subdirectory(logic)
add_subdirectory(map_io)
add_subdirectory(network)
add_subdirectory(profile)
add_subdirectory(sound)
add_subdirectory(trigger)
add_subdirectory(ui_basic)
add_subdirectory(ui_fsmenu)
add_subdirectory(wui)
add_subdirectory(scripting)

add_executable (widelands ${WL_SRCS} build_info.cc)

target_link_libraries(widelands ${PNG_LIBRARY})
target_link_libraries(widelands ${ZLIB_LIBRARY})
target_link_libraries(widelands ${JPEG_LIBRARY})
target_link_libraries(widelands ${TIFF_LIBRARY})
target_link_libraries(widelands ${SDL_LIBRARY})
target_link_libraries(widelands ${SDLIMAGE_LIBRARY})
target_link_libraries(widelands ${SDLMIXER_LIBRARY})
target_link_libraries(widelands ${SDLNET_LIBRARY})
target_link_libraries(widelands ${SDLTTF_LIBRARY})
target_link_libraries(widelands ${LUA_LIBRARY})
target_link_libraries(widelands ${SDLGFX_LIBRARY})
target_link_libraries(widelands ${Boost_unit_test_framework_LIBRARIES})

target_link_libraries(widelands widelands_io_filesystem)
target_link_libraries(widelands widelands_profile)
target_link_libraries(widelands widelands_logic)
target_link_libraries(widelands widelands_io)
target_link_libraries(widelands widelands_editor_tools)
target_link_libraries(widelands widelands_ai)
target_link_libraries(widelands widelands_game_io)
target_link_libraries(widelands widelands_graphic)
target_link_libraries(widelands widelands_sound)
target_link_libraries(widelands widelands_editor)
target_link_libraries(widelands widelands_ui_fsmenu)
target_link_libraries(widelands widelands_ui_basic)
target_link_libraries(widelands widelands_network)
target_link_libraries(widelands widelands_wui)
target_link_libraries(widelands widelands_economy)
target_link_libraries(widelands widelands_events)
target_link_libraries(widelands widelands_map_io)
target_link_libraries(widelands widelands_logic)
target_link_libraries(widelands widelands_editor_ui_menus)
target_link_libraries(widelands widelands_editor_tools)
target_link_libraries(widelands widelands_ui_basic)
target_link_libraries(widelands widelands_events)
target_link_libraries(widelands widelands_trigger)
target_link_libraries(widelands widelands_scripting)

if (APPLE)
  target_link_libraries(widelands ${INTL_LIBRARY})
endif (APPLE)

install(TARGETS widelands DESTINATION bin COMPONENT ExecutableFiles)
