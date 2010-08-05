file(GLOB WL_SCRIPTING_TEST_SRCS *.cc)

add_executable(test_widelands_scripting ${WL_SCRIPTING_TEST_SRCS} ../../build_info.cc)

set(SDL_LIBRARY_NOMAIN ${SDL_LIBRARY})
list(REMOVE_ITEM SDL_LIBRARY_NOMAIN  ${SDLMAIN_LIBRARY}) 

target_link_libraries(test_widelands_scripting 
	widelands_logic widelands_main widelands_ai widelands_scripting widelands_sound widelands_wui widelands_graphic widelands_economy widelands_map_io widelands_editor widelands_ui_fsmenu widelands_network widelands_game_io widelands_ui_basic widelands_profile widelands_io
	widelands_io_filesystem widelands_editor widelands_editor_tools widelands_editor_ui_menus 
	${Boost_LIBRARIES}
	${PNG_LIBRARY}
	${ZLIB_LIBRARY}
	${JPEG_LIBRARY}
	${TIFF_LIBRARY}
	${SDL_LIBRARY_NOMAIN}
	${SDLIMAGE_LIBRARY}
	${SDLMIXER_LIBRARY}
	${SDLNET_LIBRARY}
	${SDLTTF_LIBRARY}
	${LUA_LIBRARY}
	${SDLGFX_LIBRARY}
	)
if (GGZ_CORE_FOUND)
  target_link_libraries(test_widelands_scripting ${GGZ_CORE_LIBRARY})
endif (GGZ_CORE_FOUND)

if (APPLE)
  target_link_libraries(test_widelands_scripting ${INTL_LIBRARY})
endif (APPLE)

if (WIN32)
if (DEFINED MSVC)
  target_link_libraries(test_widelands_scripting ${INTL_LIBRARY})
  target_link_libraries(test_widelands_scripting ws2_32)
  #GGZ on MSVC using 3rdparty bundle needs expat library
  #if expat is not found, then 3rdparty bundle is not in use, so just continue.
  if (GGZ_CORE_FOUND)
    find_library(EXPAT_LIBRARY libexpat)
    if (EXPAT_LIBRARY)
      target_link_libraries(test_widelands_scripting ${EXPAT_LIBRARY})
    endif (EXPAT_LIBRARY)
  endif (GGZ_CORE_FOUND)
else (DEFINED MSVC)
  target_link_libraries(test_widelands_scripting intl)
  target_link_libraries(test_widelands_scripting wsock32)
endif (DEFINED MSVC) 
endif (WIN32)


add_test(test_widelands_scripting test_widelands_scripting)