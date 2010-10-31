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

file(GLOB_RECURSE WL_SRCS_CC *.cc)
file(GLOB_RECURSE WL_SRCS_H *.h)
file(GLOB_RECURSE WL_SRCS_CPP *.cpp)

#create WL_SRCS and WL_TEST_SRCS
#convention: any directory called "test" contains test sources, including subdirectories
set(WL_TEST_SRCS "")
foreach (sourcefile ${WL_SRCS_CC} ${WL_SRCS_H})
  if (${sourcefile} MATCHES ".*/test/.*")
    list (APPEND WL_TEST_SRCS ${sourcefile})
    list (REMOVE_ITEM WL_SRCS_CC ${sourcefile})
    list (REMOVE_ITEM WL_SRCS_H ${sourcefile})
  endif (${sourcefile} MATCHES ".*/test/.*")
endforeach (sourcefile ${WL_SRCS_CC} ${WL_SRCS_H})
#convention: main.cc should be removed from all lists
list (REMOVE_ITEM WL_SRCS_CC "${CMAKE_CURRENT_SOURCE_DIR}/main.cc")

IF (DEFINED MSVC)
  add_executable (widelands ${GUI_TYPE} main.cc build_info.cc widelands.rc)
else (DEFINED MSVC)
  add_executable (widelands ${GUI_TYPE} main.cc build_info.cc)
  set_target_properties(widelands PROPERTIES MACOSX_BUNDLE TRUE)
endif (DEFINED MSVC)

add_library (widelands_all ${WIN32_ICON_O} ${WL_SRCS_CC} ${WL_SRCS_H})

set (WL_SOURCES_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set (WL_SOURCES_BINARY_ROOT ${CMAKE_CURRENT_BINARY_DIR})

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/CppCheckSuppressions.cmake ${CMAKE_CURRENT_BINARY_DIR}/CppCheckSuppressions)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set (commands "")
  foreach(sourcefile ${WL_SRCS_CC} ${WL_SRCS_H} ${WL_SRCS_CPP})
    string(REPLACE "${CMAKE_SOURCE_DIR}/src/" "" sourcewithoutpath ${sourcefile})
    string(REGEX REPLACE "\\.|\\/" "_" sourcenoslash ${sourcewithoutpath})
    if (WIN32)
      string(REGEX REPLACE "^.*:" "" sourcenoslash ${sourcenoslash})
    endif (WIN32)
    get_filename_component(sourcename ${sourcefile} NAME) 
    set(stamp ${sourcename}_)
    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/codecheck_${sourcenoslash}"
      COMMAND ${CMAKE_COMMAND} -DWL_SOURCE_CHECKER=${CMAKE_SOURCE_DIR}/cmake/codecheck/CodeCheck.py -Dsourcefile=${sourcefile} -DCMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -Dstamp=codecheck_${sourcenoslash} -P ${CMAKE_SOURCE_DIR}/src/CodeCheck.cmake
      DEPENDS ${sourcefile}
      COMMENT "Checking ${sourcewithoutpath} with CodeCheck"
    )
    list(APPEND commands ${CMAKE_CURRENT_BINARY_DIR}/codecheck_${sourcenoslash})
    if (WL_EXTRAWARNINGS)
      add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/cppcheck_${sourcenoslash}"
	COMMAND ${CMAKE_COMMAND} -DWL_SOURCES_BINARY_ROOT=${WL_SOURCES_BINARY_ROOT} -Dincludedir=${CMAKE_SOURCE_DIR}/src -Dsourcefile=${sourcefile} -DCMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DCMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -Dstamp=cppcheck_${sourcenoslash} -P ${CMAKE_SOURCE_DIR}/src/CppCheck.cmake
	DEPENDS ${sourcefile}
	COMMENT "Checking ${sourcewithoutpath} with CppCheck"
      )
      list(APPEND commands ${CMAKE_CURRENT_BINARY_DIR}/cppcheck_${sourcenoslash})
#        set_source_files_properties(${sourcefile} PROPERTIES OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/codecheck_${stamp};${CMAKE_CURRENT_BINARY_DIR}/cppcheck_${stamp}")
    else (WL_EXTRAWARNINGS)
#        set_source_files_properties(${sourcefile} PROPERTIES OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/codecheck_${stamp}")
    endif (WL_EXTRAWARNINGS)
  endforeach(sourcefile ${WL_SRCS_CC} ${WL_SRCS_H} ${WL_SRCS_CPP})
  add_custom_target(codecheck DEPENDS ${commands})
  add_dependencies(widelands codecheck)
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

macro(LinkExecutable exe_name is_test)
  target_link_libraries(${exe_name} ${PNG_LIBRARY})
  target_link_libraries(${exe_name} ${ZLIB_LIBRARY})
  target_link_libraries(${exe_name} ${SDL_LIBRARY})
  target_link_libraries(${exe_name} ${SDLIMAGE_LIBRARY})
  target_link_libraries(${exe_name} ${SDLMIXER_LIBRARY})
  target_link_libraries(${exe_name} ${SDLNET_LIBRARY})
  target_link_libraries(${exe_name} ${SDLTTF_LIBRARY})
  target_link_libraries(${exe_name} ${LUA_LIBRARY})
  target_link_libraries(${exe_name} ${SDLGFX_LIBRARY})
  target_link_libraries(${exe_name} ${GGZ_CORE_LIBRARY})
  target_link_libraries(${exe_name} ${OPENGL_gl_LIBRARY})

  if (${is_test} STREQUAL TRUE)
    target_link_libraries(${exe_name} ${Boost_LIBRARIES} ${srces})
  endif (${is_test} STREQUAL TRUE)

  target_link_libraries(${exe_name} widelands_all)
endmacro(LinkExecutable exe_name is_test)

#the subdirectories are only used for test definition
#temporarily disabled the 'old' tests due to incompatibility
#add_subdirectory(economy)
#add_subdirectory(io)
#temporarily disabled lua test, this is a special one
#add_subdirectory(scripting)

LinkExecutable(widelands FALSE)

if (APPLE OR WIN32)
  target_link_libraries(widelands ${INTL_LIBRARY})
endif (APPLE OR WIN32)

if (WIN32)
  if (DEFINED MSVC)
    target_link_libraries(widelands ws2_32)

    #GGZ on MSVC using 3rdparty bundle needs expat library
    #if expat is not found, then 3rdparty bundle is not in use, so just continue.
    find_package(EXPAT REQUIRED)
    target_link_libraries(widelands ${EXPAT_LIBRARY})

  else (DEFINED MSVC)
    target_link_libraries(widelands wsock32)
  endif (DEFINED MSVC) 
endif (WIN32)

install(TARGETS widelands DESTINATION ${WLBUILD_INSTALL_BINDIR} COMPONENT ExecutableFiles)
