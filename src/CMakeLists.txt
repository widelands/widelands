configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/build_info.cc.cmake ${CMAKE_CURRENT_BINARY_DIR}/build_info.cc)

IF (WIN32)
  configure_file (${CMAKE_CURRENT_SOURCE_DIR}/../utils/win32/msvc/widelands.rc.cmake ${CMAKE_CURRENT_BINARY_DIR}/widelands.rc)
  if (NOT WL_VERSION_MAJOR OR WL_VERSION_MAJOR STREQUAL "")
    set (WL_VERSION_MAJOR 0)
  endif (NOT WL_VERSION_MAJOR OR WL_VERSION_MAJOR STREQUAL "")
  if (NOT WL_VERSION_MINOR OR WL_VERSION_MINOR STREQUAL "")
    set (WL_VERSION_MINOR 15)
  endif (NOT WL_VERSION_MINOR OR WL_VERSION_MINOR STREQUAL "")
  if (NOT WL_VERSION_PATCH OR WL_VERSION_PATCH STREQUAL "")
    set (WL_VERSION_PATCH 1)
  endif (NOT WL_VERSION_PATCH OR WL_VERSION_PATCH STREQUAL "")
  configure_file (${CMAKE_CURRENT_SOURCE_DIR}/../utils/win32/msvc/Widelands-msvc.iss.cmake ${CMAKE_CURRENT_BINARY_DIR}/Widelands-msvc.iss)
  IF (MINGW)
	set(WIN32_ICON_O ${CMAKE_CURRENT_BINARY_DIR}/wl_icon.o)
	ADD_CUSTOM_COMMAND( OUTPUT ${WIN32_ICON_O}
		COMMAND ${MINGW_PREFIX}windres
			-o${WIN32_ICON_O}
			-i${CMAKE_CURRENT_BINARY_DIR}/widelands.rc
		)
	ENDIF (MINGW)
endif (WIN32)

file(GLOB_RECURSE WL_SRCS_CC *.cc)
file(GLOB_RECURSE WL_SRCS_H *.h)
file(GLOB_RECURSE WL_SRCS_CPP *.cpp)

#convention: any directory called "test" contains test sources, including subdirectories
set(WL_TEST_SRCS "")
set(WL_CODECHECK_SRCS "")
foreach (sourcefile ${WL_SRCS_CC} ${WL_SRCS_H})
  if (${sourcefile} MATCHES ".*/test/.*")
    list (APPEND WL_TEST_SRCS ${sourcefile})
    list (REMOVE_ITEM WL_SRCS_CC ${sourcefile})
    list (REMOVE_ITEM WL_SRCS_H ${sourcefile})
  endif (${sourcefile} MATCHES ".*/test/.*")
  # exclude test sources from Codecheck, as well as the scripting/eris sources from lua and minizip
  if (NOT ${sourcefile} MATCHES ".*/scripting/eris/.*" AND NOT ${sourcefile} MATCHES ".*/test/.*" AND NOT ${sourcefile} MATCHES ".*/minizip/.*")
    list (APPEND WL_CODECHECK_SRCS ${sourcefile})
  endif (NOT ${sourcefile} MATCHES ".*/scripting/eris/.*" AND NOT ${sourcefile} MATCHES ".*/test/.*" AND NOT ${sourcefile} MATCHES ".*/minizip/.*")
endforeach (sourcefile ${WL_SRCS_CC} ${WL_SRCS_H})
foreach (sourcefile ${WL_SRCS_CPP})
  list (APPEND WL_CODECHECK_SRCS ${sourcefile})
endforeach (sourcefile ${WL_SRCS_CPP})
#convention: main.cc should be removed from all lists
list (REMOVE_ITEM WL_SRCS_CC "${CMAKE_CURRENT_SOURCE_DIR}/main.cc")

IF (DEFINED MSVC)
  add_executable (widelands ${GUI_TYPE} main.cc build_info.cc widelands.rc)
else (DEFINED MSVC)
  if (DEFINED MINGW)
    add_executable (widelands ${GUI_TYPE} main.cc build_info.cc ${WIN32_ICON_O})
  else (DEFINED MINGW)
    add_executable (widelands ${GUI_TYPE} main.cc build_info.cc)
  endif (DEFINED MINGW)
endif (DEFINED MSVC)

add_library (widelands_all STATIC ${WL_SRCS_CC} ${WL_SRCS_H})

set (WL_SOURCES_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set (WL_SOURCES_BINARY_ROOT ${CMAKE_CURRENT_BINARY_DIR})

if (CMAKE_BUILD_TYPE STREQUAL Debug)
  find_package (PythonInterp REQUIRED)
  set (commands "")
  foreach(sourcefile ${WL_CODECHECK_SRCS})
    string(REPLACE "${CMAKE_SOURCE_DIR}/src/" "" sourcewithoutpath ${sourcefile})
    string(REGEX REPLACE "\\.|\\/" "_" sourcenoslash ${sourcewithoutpath})
    if (WIN32)
      string(REGEX REPLACE "^.*:" "" sourcenoslash ${sourcenoslash})
    endif (WIN32)
    get_filename_component(sourcename ${sourcefile} NAME)
    set(stamp ${sourcename}_)
    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/codecheck_${sourcenoslash}"
      COMMAND ${CMAKE_COMMAND} -DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE} -DWL_SOURCE_CHECKER=${CMAKE_SOURCE_DIR}/cmake/codecheck/CodeCheck.py -Dsourcefile=${sourcefile} -DCMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -Dstamp=codecheck_${sourcenoslash} -P ${CMAKE_SOURCE_DIR}/src/CodeCheck.cmake
      DEPENDS ${sourcefile}
      COMMENT "Checking ${sourcewithoutpath} with CodeCheck"
    )
    list(APPEND commands ${CMAKE_CURRENT_BINARY_DIR}/codecheck_${sourcenoslash})
  endforeach(sourcefile ${WL_CODECHECK_SRCS})
  add_custom_target(codecheck DEPENDS ${commands})
  add_dependencies(widelands codecheck)
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

#the subdirectories are only used for test definition
#temporarily disabled the 'old' tests due to incompatibility
if (WL_UNIT_TESTS)
  # Tests need to link with SDL library without main
  set(TEST_EXTRA_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/build_info.cc)
  set(TEST_EXTRA_LIBS ${SDL_LIBRARY} )
  list(REMOVE_ITEM TEST_EXTRA_LIBS  ${SDLMAIN_LIBRARY})

  add_subdirectory(economy)
  add_subdirectory(io)
endif (WL_UNIT_TESTS)

add_subdirectory(scripting)

if (NOT MINIZIP_FOUND)
  add_subdirectory(minizip)
  target_link_libraries(widelands_all wl_minizip)
endif (NOT MINIZIP_FOUND)

target_link_libraries(widelands_all eris)
target_link_libraries(widelands_all ${SDLIMAGE_LIBRARY})
target_link_libraries(widelands_all ${SDLMIXER_LIBRARY})
target_link_libraries(widelands_all ${SDLNET_LIBRARY})
target_link_libraries(widelands_all ${SDLTTF_LIBRARY})
target_link_libraries(widelands_all ${SDLGFX_LIBRARY})
target_link_libraries(widelands_all ${PNG_LIBRARY})
target_link_libraries(widelands_all ${ZLIB_LIBRARY})
target_link_libraries(widelands_all ${OPENGL_gl_LIBRARY})
target_link_libraries(widelands_all ${GLEW_LIBRARY})
target_link_libraries(widelands_all ${Boost_SIGNALS_LIBRARY})

if (MINIZIP_FOUND)
  target_link_libraries(widelands_all ${MINIZIP_LIBRARY})
endif (MINIZIP_FOUND)
if (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
  target_link_libraries(widelands_all ${EXECINFO_LIBRARY})
endif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")

if (DEFINED WL_EXTRA_LINK_LIBRARIES)
  target_link_libraries(widelands_all ${WL_EXTRA_LINK_LIBRARIES})
endif (DEFINED WL_EXTRA_LINK_LIBRARIES)

target_link_libraries(widelands widelands_all)
target_link_libraries(widelands ${SDL_LIBRARY})

if (APPLE OR WIN32 OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  target_link_libraries(widelands_all ${INTL_LIBRARY})
endif (APPLE OR WIN32 OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")

if (WIN32)
  if (DEFINED MSVC)
    target_link_libraries(widelands_all ws2_32)

    #gettext on MSVC using 3rdparty bundle needs iconv library
    find_library(ICONV_LIBRARY iconv libiconv REQUIRED)
    target_link_libraries(widelands_all ${ICONV_LIBRARY})

  else (DEFINED MSVC)
    target_link_libraries(widelands_all wsock32)
  endif (DEFINED MSVC)
endif (WIN32)

install(TARGETS widelands DESTINATION ${WL_INSTALL_BINDIR} COMPONENT ExecutableFiles)
