configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/build_info.cc.cmake ${CMAKE_CURRENT_BINARY_DIR}/build_info.cc)

IF (WIN32)
  configure_file (${CMAKE_CURRENT_SOURCE_DIR}/../build/win32/msvc/widelands.rc.cmake ${CMAKE_CURRENT_BINARY_DIR}/widelands.rc)
  if (NOT WL_VERSION_MAJOR OR WL_VERSION_MAJOR STREQUAL "")
    set (WL_VERSION_MAJOR 0)
  endif (NOT WL_VERSION_MAJOR OR WL_VERSION_MAJOR STREQUAL "")
  if (NOT WL_VERSION_MINOR OR WL_VERSION_MINOR STREQUAL "")
    set (WL_VERSION_MINOR 15)
  endif (NOT WL_VERSION_MINOR OR WL_VERSION_MINOR STREQUAL "")
  if (NOT WL_VERSION_PATCH OR WL_VERSION_PATCH STREQUAL "")
    set (WL_VERSION_PATCH 1)
  endif (NOT WL_VERSION_PATCH OR WL_VERSION_PATCH STREQUAL "")
  configure_file (${CMAKE_CURRENT_SOURCE_DIR}/../build/win32/msvc/Widelands-msvc.iss.cmake ${CMAKE_CURRENT_BINARY_DIR}/Widelands-msvc.iss)
  IF (MINGW)
	set(WIN32_ICON_O ${CMAKE_CURRENT_BINARY_DIR}/wl_icon.o)
	ADD_CUSTOM_COMMAND( OUTPUT ${WIN32_ICON_O}
		COMMAND ${MINGW_PREFIX}windres
			-o${WIN32_ICON_O}
			-i${CMAKE_CURRENT_BINARY_DIR}/widelands.rc
		)
	ENDIF (MINGW)
endif (WIN32)

file(GLOB WL_MAIN_SRCS *.cc)

IF (DEFINED MSVC)
  add_executable (widelands ${GUI_TYPE} main.cc build_info.cc widelands.rc)
else (DEFINED MSVC)
  add_executable (widelands ${GUI_TYPE} main.cc build_info.cc ${WIN32_ICON_O})
  set_target_properties(widelands PROPERTIES MACOSX_BUNDLE TRUE)
endif (DEFINED MSVC)

file(GLOB_RECURSE WL_SRCS_ALL *.cc;*.h)

add_library(widelands_main ${WL_MAIN_SRCS})

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
set_directory_properties (PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/codecheck-stamps)

set (WL_SOURCES_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set (WL_SOURCES_BINARY_ROOT ${CMAKE_CURRENT_BINARY_DIR})

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/CppCheckSuppressions.cmake ${CMAKE_CURRENT_BINARY_DIR}/CppCheckSuppressions)

macro(CreateCodeCheck link_target)
  if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set (commands "")
    file(GLOB srcfiles *.cc)
    foreach(sourcefile ${srcfiles})
      string(REPLACE "${CMAKE_SOURCE_DIR}/src/" "" sourcewithoutpath ${sourcefile})
      string(REGEX REPLACE "\\.|\\/" "_" sourcenoslash ${sourcewithoutpath})
      if (WIN32)
	string(REGEX REPLACE "^.*:" "" sourcenoslash ${sourcenoslash})
      endif (WIN32)
      get_filename_component(sourcename ${sourcefile} NAME) 
      set(stamp ${sourcename}_)
      set(PREFIX_WL_SRCS ${PREFIX_WL_SRCS};${CMAKE_CURRENT_BINARY_DIR}/${stamp})
      add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/codecheck_${stamp}"
        COMMAND cmake -DWL_SOURCE_CHECKER=${CMAKE_SOURCE_DIR}/cmake/codecheck/CodeCheck.py -Dsourcefile=${sourcefile} -DCMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -Dstamp=codecheck_${stamp} -P ${CMAKE_SOURCE_DIR}/src/CodeCheck.cmake
        DEPENDS ${sourcefile}
        COMMENT "Checking ${sourcewithoutpath} with CodeCheck"
      )
      list(APPEND commands ${CMAKE_CURRENT_BINARY_DIR}/codecheck_${stamp})
      if (WL_EXTRAWARNINGS)
        add_custom_command(
          OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/cppcheck_${stamp}"
          COMMAND cmake -DWL_SOURCES_BINARY_ROOT=${WL_SOURCES_BINARY_ROOT} -Dincludedir=${CMAKE_SOURCE_DIR}/src -Dsourcefile=${sourcefile} -DCMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DCMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -Dstamp=cppcheck_${stamp} -P ${CMAKE_SOURCE_DIR}/src/CppCheck.cmake
          DEPENDS ${sourcefile}
          COMMENT "Checking ${sourcewithoutpath} with CppCheck"
        )
        list(APPEND commands ${CMAKE_CURRENT_BINARY_DIR}/cppcheck_${stamp})
#        set_source_files_properties(${sourcefile} PROPERTIES OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/codecheck_${stamp};${CMAKE_CURRENT_BINARY_DIR}/cppcheck_${stamp}")
      else (WL_EXTRAWARNINGS)
#        set_source_files_properties(${sourcefile} PROPERTIES OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/codecheck_${stamp}")
      endif (WL_EXTRAWARNINGS)
    endforeach(sourcefile ${srcfiles})
    add_custom_target(codecheck_${link_target} DEPENDS ${commands})
    add_dependencies(${link_target} codecheck_${link_target})
  endif (CMAKE_BUILD_TYPE STREQUAL Debug)
endmacro(CreateCodeCheck)

CreateCodeCheck(widelands_main)

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
add_subdirectory(scripting)
add_subdirectory(sound)
add_subdirectory(trigger)
add_subdirectory(ui_basic)
add_subdirectory(ui_fsmenu)
add_subdirectory(wui)

target_link_libraries(widelands widelands_main)

target_link_libraries(widelands widelands_ai widelands_profile widelands_logic widelands_economy)
target_link_libraries(widelands widelands_economy widelands_ai widelands_map_io widelands_logic widelands_ui_basic)
target_link_libraries(widelands widelands_editor widelands_trigger widelands_logic widelands_map_io widelands_events widelands_editor_ui_menus widelands_editor_tools widelands_graphic widelands_ui_basic widelands_profile widelands_ui_fsmenu widelands_wui widelands_io)
target_link_libraries(widelands widelands_editor_tools widelands_ui_basic widelands_graphic widelands_logic widelands_wui)
target_link_libraries(widelands widelands_editor_ui_menus widelands_map_io widelands_events widelands_graphic widelands_ui_basic widelands_profile widelands_logic widelands_editor_tools widelands_wui widelands_ui_fsmenu widelands_trigger widelands_io)
target_link_libraries(widelands widelands_events widelands_map_io widelands_wui widelands_ui_basic widelands_profile widelands_logic widelands_editor widelands_graphic widelands_trigger widelands_economy)
target_link_libraries(widelands widelands_game_io widelands_map_io widelands_wui widelands_profile widelands_logic widelands_economy widelands_io)
target_link_libraries(widelands widelands_graphic widelands_sound widelands_wui widelands_logic widelands_economy widelands_ui_basic widelands_profile widelands_io)
target_link_libraries(widelands widelands_io widelands_io_filesystem)
target_link_libraries(widelands widelands_io_filesystem)
target_link_libraries(widelands widelands_logic widelands_main widelands_ai widelands_scripting widelands_sound widelands_wui widelands_graphic widelands_economy widelands_map_io widelands_events widelands_editor widelands_trigger widelands_ui_fsmenu widelands_network widelands_game_io widelands_ui_basic widelands_profile widelands_io)
target_link_libraries(widelands widelands_map_io widelands_wui widelands_trigger widelands_events widelands_graphic widelands_logic widelands_profile widelands_economy widelands_io)
target_link_libraries(widelands widelands_network widelands_ui_basic widelands_profile widelands_logic widelands_ui_fsmenu widelands_wui widelands_io)
target_link_libraries(widelands widelands_profile widelands_logic widelands_io)
target_link_libraries(widelands widelands_scripting widelands_wui widelands_logic)
target_link_libraries(widelands widelands_sound widelands_wui widelands_profile widelands_logic widelands_io)
target_link_libraries(widelands widelands_trigger widelands_events widelands_map_io widelands_logic widelands_editor widelands_profile)
target_link_libraries(widelands widelands_ui_basic widelands_logic widelands_economy widelands_graphic widelands_sound widelands_profile widelands_io widelands_ui_fsmenu)
target_link_libraries(widelands widelands_ui_fsmenu widelands_logic widelands_sound widelands_wui widelands_graphic widelands_game_io widelands_network widelands_map_io widelands_ui_basic widelands_profile widelands_io)
target_link_libraries(widelands widelands_wui widelands_sound widelands_profile widelands_logic widelands_trigger widelands_editor widelands_graphic widelands_economy widelands_game_io widelands_ui_fsmenu widelands_ui_basic widelands_io)

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

if (GGZ_CORE_FOUND)
  target_link_libraries(widelands ${GGZ_CORE_LIBRARY})
endif (GGZ_CORE_FOUND)

if (APPLE)
  target_link_libraries(widelands ${INTL_LIBRARY})
endif (APPLE)

if (WIN32)
if (DEFINED MSVC)
  target_link_libraries(widelands ${INTL_LIBRARY})
  target_link_libraries(widelands ws2_32)
  #GGZ on MSVC using 3rdparty bundle needs expat library
  #if expat is not found, then 3rdparty bundle is not in use, so just continue.
  if (GGZ_CORE_FOUND)
    find_library(EXPAT_LIBRARY libexpat)
    if (EXPAT_LIBRARY)
      target_link_libraries(widelands ${EXPAT_LIBRARY})
    endif (EXPAT_LIBRARY)
  endif (GGZ_CORE_FOUND)
else (DEFINED MSVC)
  target_link_libraries(widelands intl)
  target_link_libraries(widelands wsock32)
endif (DEFINED MSVC) 
endif (WIN32)

install(TARGETS widelands DESTINATION ${WLBUILD_INSTALL_BINDIR} COMPONENT ExecutableFiles)
