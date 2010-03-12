project (widelands)

# possible configurations:
# set them via cmake -DVARIABLE=VALUE
# CMAKE_BUILD_TYPE:
#     Default is "Debug", Distributors should use "Release"
# WL_VERSION_STANDARD
#     Uses the defined standard values for versioning ("BuildXX.YY") if defined.
#     (Should be set for each official release below.)
# WL_VERSION_MAJOR, WL_VERSION_MINOR (, WL_VERSION_PATCH):
#     Default is using SVN revision, Distributors should use correct version
#     Attention: We are currently only using MAJOR and MINOR.
#     If you try to set WL_VERSION directly, it will ignore your setting.
# WL_VERSION_ADDITION:
#     Suffix of the Build version (e.g. bzr, DEV, RC1, ...)
# CMAKE_INSTALL_PREFIX:
#     Prefix for installation of the files.
#     This is not the install dir which is used for config.h - see WL_INSTALL_PREFIX
#     default: /usr/local
# WL_INSTALL_PREFIX:
#     Prefix for INSTALL_PREFIX in config.h.
#     This is not the install dir for make install - see CMAKE_INSTALL_PREFIX
#     default: .
# WL_INSTALL_BINDIR:
#     Directory the binary should be in (relative to WL_INSTALL_PREFIX)
#     default: games
# WL_INSTALL_DATADIR
#     Directory for the Widelands data (relative to WL_INSTALL_PREFIX)
#     default: share/games/widelands
# WL_INSTALL_LOCALEDIR
#     Directory for Widelands locales (relative to WL_INSTALL_PREFIX)
#     default: share/games/widelands/locale

cmake_minimum_required (VERSION 2.6)


# Used as versioning, if WL_VERSION_STANDARD is definded.
if (WL_VERSION_STANDARD STREQUAL "TRUE")
	# Set the values below for standard versioning.
	# As long as WL_VERSION_MAJOR is 0, the naming will be in form of
	# "Build${WL_VERSION_MINOR}" + ".${WL_VERSION_PATCH}" if WL_VERSION_PATCH is
	# defined.
	set (WL_VERSION_MAJOR 0)
	set (WL_VERSION_MINOR 15)

	# Just define in case that we release patches for official builds
	#set (WL_VERSION_PATCH 90)

	# An addition to the version name (e.g. RC1, DEV, ...)
	set (WL_VERSION_ADDITION DEV)
endif (WL_VERSION_STANDARD STREQUAL "TRUE")


# Define the directory structure for installation - will be hardcoded in WL bin
# If nothing is given on command line, set the standard values.

# General install prefix for config.h
if (NOT WL_INSTALL_PREFIX OR WL_INSTALL_PREFIX STREQUAL "")
	set (WL_INSTALL_PREFIX .)
endif (NOT WL_INSTALL_PREFIX OR WL_INSTALL_PREFIX STREQUAL "")

# Widelands binary directory
if (NOT WL_INSTALL_BINDIR OR WL_INSTALL_BINDIR STREQUAL "")
	set (WL_INSTALL_BINDIR games)
endif (NOT WL_INSTALL_BINDIR OR WL_INSTALL_BINDIR STREQUAL "")

# Widelands data directory
if (NOT WL_INSTALL_DATADIR OR WL_INSTALL_DATADIR STREQUAL "")
	set (WL_INSTALL_DATADIR share/games/widelands)
endif (NOT WL_INSTALL_DATADIR OR WL_INSTALL_DATADIR STREQUAL "")

# Widelands locale directory
if (NOT WL_INSTALL_LOCALEDIR OR WL_INSTALL_LOCALEDIR STREQUAL "")
	set (WL_INSTALL_LOCALEDIR share/games/widelands/locale)
endif (NOT WL_INSTALL_LOCALEDIR OR WL_INSTALL_LOCALEDIR STREQUAL "")

# is this needed?
set (WL_SOURCE_CHECKER ${CMAKE_CURRENT_SOURCE_DIR}/cmake/codecheck/CodeCheck.py)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Build-directory and source-directory are the same!")
endif (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

if (CMAKE_INSTALL_PREFIX STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Build-directory and install-directory are the same!")
endif (CMAKE_INSTALL_PREFIX STREQUAL CMAKE_BINARY_DIR)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

if (CMAKE_BUILD_TYPE STREQUAL "")
  set (CMAKE_BUILD_TYPE Debug)
endif (CMAKE_BUILD_TYPE STREQUAL "")

set (CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})


# CMAKE only defines "-g", but we need -DDEBUG also, and we need -DNOPARACHUTE (for SDL) in Debug
set (CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG -DNOPARACHUTE" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)

# CMAKE defines "-O3 -DNDEBUG", but we better say -O2 (see gcc manual)
set (CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)


#If building with MSVC, then check for 3rdparty libs
if (DEFINED MSVC)
  set (WL_3RDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty" CACHE STRING "Widelands 3rdparty dir")
  set (CMAKE_INCLUDE_PATH "${WL_3RDPARTY_DIR}/include;${WL_3RDPARTY_DIR}/include/sdl")
  set (CMAKE_LIBRARY_PATH "${WL_3RDPARTY_DIR}/lib")

  add_definitions( -Dand=&& -Dor=|| -Dnot=! -Dxor=^ )
  add_definitions( -Dstrcasecmp=_stricmp )
  add_definitions( -Dstrncasecmp=_strnicmp )
  add_definitions( -Dsnprintf=_snprintf )
  add_definitions( -Drint=floor )
  add_definitions( -Dstrtoll=_strtoi64 )
  add_definitions( -Dstrtoull=_strtoui64 )
  add_definitions( -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS )
  add_definitions( -Dssize_t=size_t )
  add_definitions( -Dmkdir=_mkdir )


  find_library(INTL_LIBRARY NAMES intl libintl)

  # CMAKE only defines "-g", but we need -DDEBUG also, and we need -DNOPARACHUTE (for SDL) in Debug
  set (CMAKE_CXX_FLAGS_DEBUG " -DDEBUG -DNOPARACHUTE /MDd /Zi /D_DEBUG" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)

  # CMAKE defines "-O3 -DNDEBUG", but we better say -O2 (see gcc manual)
  set (CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG /MD" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)

  set (CMAKE_EXE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:YES /NODEFAULTLIB:msvcrt.lib" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)

else (DEFINED MSVC)
  set (HAVE_VARARRAY TRUE CACHE BOOL "Use variable length arrays to improve performance. Requires gcc." )
endif (DEFINED MSVC)

if (DEFINED WL_3RDPARTY_DIR)
  set (CMAKE_INCLUDE_PATH "${WL_3RDPARTY_DIR}/include;${WL_3RDPARTY_DIR}/include/sdl")
  set (CMAKE_LIBRARY_PATH "${WL_3RDPARTY_DIR}/lib")
endif (DEFINED WL_3RDPARTY_DIR)

find_package(GGZ_Core)

if (GGZ_CORE_FOUND)
  include_directories(${GGZ_CORE_INCLUDE_DIR})
  add_definitions("-DUSE_GGZ")
else (GGZ_CORE_FOUND)
  message ("GGZ support disabled, check your libraries path and installed packages!")
endif (GGZ_CORE_FOUND)

# we don't want someone to set this directly...
# ...and we don't want to remember from former runs...
unset (WL_VERSION)
unset (WL_VERSION CACHE)

# ...but using MAJOR, MINOR and PATCH
if (DEFINED WL_VERSION_MAJOR)
	set (CPACK_PACKAGE_VERSION_MAJOR ${WL_VERSION_MAJOR})

	# Until 1.x.x set standard VERSION => "BuildXX"
	if (WL_VERSION_MAJOR STREQUAL "0")
		if (DEFINED WL_VERSION_MINOR)
			set (WL_VERSION Build${WL_VERSION_MINOR})
			set (CPACK_PACKAGE_VERSION_MINOR ${WL_VERSION_MINOR})
			if (DEFINED WL_VERSION_PATCH)
				set (WL_VERSION ${WL_VERSION}.${WL_VERSION_PATCH})
				set (CPACK_PACKAGE_VERSION_PATCH ${WL_VERSION_PATCH})
			else (DEFINED WL_VERSION_PATCH)
				set (CPACK_PACKAGE_VERSION_PATCH 0)
			endif (DEFINED WL_VERSION_PATCH)
			if (DEFINED WL_VERSION_ADDITION)
				set (WL_VERSION ${WL_VERSION}${WL_VERSION_ADDITION})
			endif (DEFINED WL_VERSION_ADDITION)
		else (DEFINED WL_VERSION_MINOR)
			set (WL_VERSION "NO BUILD ID")
		endif (DEFINED WL_VERSION_MINOR)

	# from 1.x.x on:
	else (WL_VERSION_MAJOR STREQUAL "0")
		set (WL_VERSION ${WL_VERSION_MAJOR})
		if (DEFINED WL_VERSION_MINOR)
			set (WL_VERSION ${WL_VERSION}.${WL_VERSION_MINOR})
			set (CPACK_PACKAGE_VERSION_MINOR ${WL_VERSION_MINOR})
			if (DEFINED WL_VERSION_PATCH)
				set (WL_VERSION ${WL_VERSION}.${WL_VERSION_PATCH})
				set (CPACK_PACKAGE_VERSION_PATCH ${WL_VERSION_PATCH})
			else (DEFINED WL_VERSION_PATCH)
				set (CPACK_PACKAGE_VERSION_PATCH 0)
			endif (DEFINED WL_VERSION_PATCH)
			if (DEFINED WL_VERSION_ADDITION)
				set (WL_VERSION ${WL_VERSION}${WL_VERSION_ADDITION})
			endif (DEFINED WL_VERSION_ADDITION)
		else (DEFINED WL_VERSION_MINOR)
			set (CPACK_PACKAGE_VERSION_MINOR 0)
		endif (DEFINED WL_VERSION_MINOR)
	endif (WL_VERSION_MAJOR STREQUAL "0")
endif (DEFINED WL_VERSION_MAJOR)

if (NOT DEFINED WL_VERSION)
#  find_package (PythonInterp REQUIRED)
  execute_process (
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/utils/detect_revision.py
    OUTPUT_VARIABLE WL_VERSION
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif (NOT DEFINED WL_VERSION)
message ("Version of Widelands Build is ${WL_VERSION}(${CMAKE_BUILD_TYPE})")

file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/VERSION "${WL_VERSION}")
install (
  FILES ${CMAKE_CURRENT_BINARY_DIR}/VERSION
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT CoreVersionFile
)

include_directories(src ${CMAKE_CURRENT_BINARY_DIR}/src)

include(CheckIncludeFile)

set (BUILD_SHARED_LIBS OFF)
set (Boost_USE_STATIC_LIBS   ON)
set (Boost_USE_MULTITHREADED ON)
find_package(Boost 1.35.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(widelands ${Boost_LIBRARIES})

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

IF (NOT WIN32)
  find_package(JPEG REQUIRED)
  include_directories(${JPEG_INCLUDE_DIR})

  find_package(TIFF REQUIRED)
  include_directories(${TIFF_INCLUDE_DIR})
ENDIF (NOT WIN32)

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})

find_package(Lua51 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

if (APPLE)
  find_package(intl REQUIRED)
  include_directories(${INTL_INCLUDE_DIR})
endif (APPLE)

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

find_package(SDL_image REQUIRED)
include_directories(${SDLIMAGE_INCLUDE_DIR})

find_package(SDL_mixer REQUIRED)
include_directories(${SDLMIXER_INCLUDE_DIR})

find_package(SDL_net REQUIRED)
include_directories(${SDLNET_INCLUDE_DIR})

find_package(SDL_ttf REQUIRED)
include_directories(${SDLTTF_INCLUDE_DIR})

find_package(SDL_gfx REQUIRED)
include_directories(${SDLGFX_INCLUDE_DIR})

find_package(Lua51 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

IF (WIN32)
  SET(GUI_TYPE WIN32)
ENDIF (WIN32)

#configure_file (src/config.h.cmake src/config.h)
#configure_file (src/build_info.cc.cmake src/build_info.cc)

add_subdirectory(doc)
#add_subdirectory(po)
#add_subdirectory(pics)
add_subdirectory(src)

# install files to the correct locations here
install(
  DIRECTORY
    campaigns
    fonts
    global
    maps
    music
    sound
    tribes
    txts
    worlds
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT CoreDataFiles
  PATTERN "*.svn*" EXCLUDE
  PATTERN "SCons*" EXCLUDE
)

install(
  DIRECTORY
    doc
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug
  COMPONENT DocFiles
  PATTERN "*.svn*" EXCLUDE
  PATTERN "SCons*" EXCLUDE
)

execute_process (COMMAND cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/locale)
find_package (PythonInterp REQUIRED)
add_custom_target (
  lang ALL
#  COMMAND cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/../locale
  COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/po ${CMAKE_CURRENT_BINARY_DIR}/po
  COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/campaigns ${CMAKE_CURRENT_BINARY_DIR}/campaigns
  COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/maps ${CMAKE_CURRENT_BINARY_DIR}/maps
  COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/tribes ${CMAKE_CURRENT_BINARY_DIR}/tribes
  COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/txts ${CMAKE_CURRENT_BINARY_DIR}/txts
  COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/worlds ${CMAKE_CURRENT_BINARY_DIR}/worlds
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/utils/buildlocale.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(
  DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/locale/
  DESTINATION ${WL_INSTALL_LOCALEDIR}
  CONFIGURATIONS Debug;Release
  COMPONENT CoreLanguageFiles
  PATTERN "*.svn*" EXCLUDE
  PATTERN "SCons*" EXCLUDE
)

execute_process (COMMAND cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/pics)
add_custom_target (
  optimizepics 
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/pics
  COMMAND cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DWL_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/pics -DWL_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/pics -P PicsFunctions.cmake
)
add_custom_target (
  pics ALL 
  COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/pics ${CMAKE_CURRENT_BINARY_DIR}/pics
)

install(
  DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/pics
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT CorePicsFiles
  PATTERN "*.svn*" EXCLUDE
  PATTERN "SCons*" EXCLUDE
)


set (CPACK_PACKAGE_FILE_NAME widelands-${WL_VERSION})
set (CPACK_COMPONENTS_ALL ExecutableFiles CoreDataFiles DocFiles CoreLanguageFiles CorePicsFiles CoreVersionFile)
set (CPACK_GENERATOR TGZ)
set (CPACK_SOURCE_GENERATOR TGZ)
include(CPack)
