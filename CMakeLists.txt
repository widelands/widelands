project (widelands)

cmake_minimum_required (VERSION 2.6)

# Used as versioning, if WL_VERSION_STANDARD is definded.
# TODO(sirver): This is redundant and should not be used anywhere. We have a
# script that determines the version.
if (WL_VERSION_STANDARD)
	# Set the values below for standard versioning.
	# As long as WL_VERSION_MAJOR is 0, the naming will be in form of
	# "Build${WL_VERSION_MINOR}" + ".${WL_VERSION_PATCH}" if WL_VERSION_PATCH is
	# defined.
	set (WL_VERSION_MAJOR 0)
	set (WL_VERSION_MINOR 19)

	# Just define in case that we release patches for official builds
	#set (WL_VERSION_PATCH 90)

	# An addition to the version name (e.g. RC1, DEV, ...)
	set (WL_VERSION_ADDITION DEV)
endif (WL_VERSION_STANDARD)

# Define the directory structure for installation - will be hardcoded in WL bin
# If nothing is given on command line, set the standard values depending on platform.

if (WIN32 OR APPLE OR WL_PORTABLE)
  if (NOT WL_INSTALL_PREFIX OR WL_INSTALL_PREFIX STREQUAL "")
    set (WL_INSTALL_PREFIX ".")
  endif (NOT WL_INSTALL_PREFIX OR WL_INSTALL_PREFIX STREQUAL "")

  if (NOT WL_INSTALL_DATADIR OR WL_INSTALL_DATADIR STREQUAL "")
    set (WL_INSTALL_DATADIR ".")
  endif (NOT WL_INSTALL_DATADIR OR WL_INSTALL_DATADIR STREQUAL "")

  if (NOT WL_INSTALL_LOCALEDIR OR WL_INSTALL_LOCALEDIR STREQUAL "")
    set (WL_INSTALL_LOCALEDIR "locale")
  endif (NOT WL_INSTALL_LOCALEDIR OR WL_INSTALL_LOCALEDIR STREQUAL "")

  if (NOT WL_INSTALL_BINDIR OR WL_INSTALL_BINDIR STREQUAL "")
    set (WL_INSTALL_BINDIR ".")
  endif (NOT WL_INSTALL_BINDIR OR WL_INSTALL_BINDIR STREQUAL "")
elseif (WIN32 OR APPLE OR WL_PORTABLE)
  if (NOT WL_INSTALL_PREFIX OR WL_INSTALL_PREFIX STREQUAL "")
    set (WL_INSTALL_PREFIX "/usr/local")
  endif (NOT WL_INSTALL_PREFIX OR WL_INSTALL_PREFIX STREQUAL "")

  if (NOT WL_INSTALL_DATADIR OR WL_INSTALL_DATADIR STREQUAL "")
    set (WL_INSTALL_DATADIR "share/games/widelands")
  endif (NOT WL_INSTALL_DATADIR OR WL_INSTALL_DATADIR STREQUAL "")

  if (NOT WL_INSTALL_LOCALEDIR OR WL_INSTALL_LOCALEDIR STREQUAL "")
    set (WL_INSTALL_LOCALEDIR "${WL_INSTALL_PREFIX}/${WL_INSTALL_DATADIR}/locale")
  endif (NOT WL_INSTALL_LOCALEDIR OR WL_INSTALL_LOCALEDIR STREQUAL "")

  if (NOT WL_INSTALL_BINDIR OR WL_INSTALL_BINDIR STREQUAL "")
    set (WL_INSTALL_BINDIR "games")
  endif (NOT WL_INSTALL_BINDIR OR WL_INSTALL_BINDIR STREQUAL "")
endif (WIN32 OR APPLE OR WL_PORTABLE)

# General install prefix for config.h
if (NOT WL_INSTALL_PREFIX OR WL_INSTALL_PREFIX STREQUAL "")
	set (WL_INSTALL_PREFIX /usr/local)
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
	set (WL_INSTALL_LOCALEDIR ${WL_INSTALL_PREFIX}/${WL_INSTALL_DATADIR}/locale)
endif (NOT WL_INSTALL_LOCALEDIR OR WL_INSTALL_LOCALEDIR STREQUAL "")

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Build-directory and source-directory are the same!")
endif (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

if (CMAKE_INSTALL_PREFIX STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Build-directory and install-directory are the same!")
endif (CMAKE_INSTALL_PREFIX STREQUAL CMAKE_BINARY_DIR)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  find_path(FILE_WL_RELEASE "WL_RELEASE" ${CMAKE_CURRENT_SOURCE_DIR})
  if (${FILE_WL_RELEASE} STREQUAL "FILE_WL_RELEASE-NOTFOUND")
    set (CMAKE_BUILD_TYPE Debug)
  else (${FILE_WL_RELEASE} STREQUAL "FILE_WL_RELEASE-NOTFOUND")
    set (CMAKE_BUILD_TYPE Release)
  endif (${FILE_WL_RELEASE} STREQUAL "FILE_WL_RELEASE-NOTFOUND")
endif (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")

set (CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})

# Unit tests are built per default in Debug and not build in Release
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  option (WL_UNIT_TESTS "build and run unit tests (requires boost_unit_test_framework)" ON)
else (CMAKE_BUILD_TYPE STREQUAL "Debug")
  option (WL_UNIT_TESTS "build and run unit tests (requires boost_unit_test_framework)" OFF)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

# we only include Boost Headers to the main executable, no libraries
# unit_test_framework is for testing only
if (WL_UNIT_TESTS)
  message(STATUS "Enabled unit tests")
  if (DEFINED MSVC)
    set (BUILD_SHARED_LIBS OFF)
	set (Boost_USE_STATIC_RUNTIME ON)
	set (Boost_USE_STATIC_LIBS ON)
  else (DEFINED MSVC)
    set (Boost_USE_STATIC_LIBS  OFF)
  endif (DEFINED MSVC)
  set (Boost_FIND_COMPONENTS unit_test_framework)
  set (Boost_USE_MULTITHREADED ON)
  set (Boost_DETAILED_FAILURE_MSG ON)
  find_package(Boost 1.48 COMPONENTS unit_test_framework REQUIRED)

  if (NOT DEFINED MSVC)
  # If boost unit test library is linked dynamically, BOOST_TEST_DYN_LINK must be defined
    string( REGEX MATCH ".a$" BOOST_STATIC_UNIT_TEST_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    if (NOT BOOST_STATIC_UNIT_TEST_LIB)
      add_definitions(-DBOOST_TEST_DYN_LINK)
    endif (NOT BOOST_STATIC_UNIT_TEST_LIB)
  endif(NOT DEFINED MSVC)

  include(CTest)
  enable_testing()

  #automatic unit testing hook, must be updated to reflect all tests
  #if this is too intricate, we can also have this as an automatic detection
  #but the current way allows to disable certain checks if they are taking
  #too much time when ran every compile time
  add_custom_target(autotest ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS widelands;test_widelands_scripting;test_economy;test_io_filesystem
  )
else (WL_UNIT_TESTS)
  message(STATUS "Disabled Unit Tests")
  find_package(Boost 1.48 REQUIRED)
endif (WL_UNIT_TESTS)

option (WL_STRICT "If strict compilation mode is requested (almost every warning is an error)" OFF)

set (PARAMETER_COMPILERFLAG_OLDSTYLECAST_EXTENDED "-Wold-style-cast -isystem ${Boost_INCLUDE_DIR}")
set (PARAMETER_COMPILERFLAG_OLDSTYLECAST "-Wold-style-cast")
set (PARAMETER_COMPILERFLAG_GENERICWARNINGS "-Wall -Wextra")
set (PARAMETER_COMPILERFLAG_EXTRAWARNINGS "-Wformat -Wformat-nonliteral -Wformat-security -Wformat-y2k -Winit-self -Winvalid-pch -Wmissing-include-dirs -Woverlength-strings -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-promo -Wswitch-default -Wundef -Wunused -Wunused-macros")
set (PARAMETER_COMPILERFLAG_GCCWARNINGS "-Wlogical-op -Wsync-nand -Wtrampolines")
set (PARAMETER_COMPILERFLAG_STRICT "-Werror -Wno-error=old-style-cast -Wno-error=deprecated-declarations -fdiagnostics-show-option")
include(CheckCXXCompilerFlag) #this include should be safe

# Enable c++11 features.
if (NOT WIN32)
  set (PARAMETER_COMPILERFLAG_CXXSTD "-std=c++11")
else()
  set (PARAMETER_COMPILERFLAG_CXXSTD "-std=gnu++11")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  EXECUTE_PROCESS(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  if (GCC_VERSION VERSION_LESS 4.7)
    if (GCC_VERSION VERSION_LESS 4.6)
      if (NOT WIN32)
        set (PARAMETER_COMPILERFLAG_CXXSTD "-std=c++0x -Dnullptr=NULL")
      else()
        set (PARAMETER_COMPILERFLAG_CXXSTD "-std=gnu++0x -Dnullptr=NULL")
      endif()
    else()
      if (NOT WIN32)
        set (PARAMETER_COMPILERFLAG_CXXSTD "-std=c++0x")
      else()
        set (PARAMETER_COMPILERFLAG_CXXSTD "-std=gnu++0x")
      endif()
    endif()
  endif()
endif()

CHECK_CXX_COMPILER_FLAG(${PARAMETER_COMPILERFLAG_CXXSTD} Compiler_std_supported)
IF (Compiler_std_supported)
  set (WL_COMPILERFLAG_CXXSTD " ${PARAMETER_COMPILERFLAG_CXXSTD}") #the space is on purpose!
ENDIF (Compiler_std_supported)


CHECK_CXX_COMPILER_FLAG(${PARAMETER_COMPILERFLAG_OLDSTYLECAST_EXTENDED} Compiler_old-style-cast_supported)
IF (Compiler_old-style-cast_supported)
  EXEC_PROGRAM(${CMAKE_CXX_COMPILER}
    ARGS --version
    OUTPUT_VARIABLE WLBUILD_COMPILERVERSION
  )
  STRING(REGEX REPLACE ".*([0-9])\\.([0-9])\\.([0-9]).*" "\\1.\\2.\\3" WLBUILD_COMPILERVERSION_REP ${WLBUILD_COMPILERVERSION})
  IF (WLBUILD_COMPILERVERSION_REP STREQUAL "4.3.2" OR WLBUILD_COMPILERVERSION_REP STREQUAL "4.3.1")
    message("===== ATTENTION ===================================================")
    message("Your compiler has been identified as GCC 4.3.1 or GCC 4.3.2")
    message(" ")
    message("There is a bug regarding this version, see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=38000")
    message("and https://bugs.launchpad.net/widelands/+bug/549479 for details.")
    message("Build will disable -isystem usage, expect lots of warnings from Boost header files.")
    message("Widelands should still compile and link correctly.")
    message("If the compiler identification is correct, please consider updating your gcc.")
    message("This warning message will disappear at GCC 4.3.3 and higher.")
    message("If you feel this is wrong, please submit a bug with widelands (not with GCC!)")
    message("and add the following information how cmake sees your C++ compiler:")
    message(" ")
    message("--------------------------------")
    message("${WLBUILD_COMPILERVERSION}")
    message("--------------------------------")
    message(" ")
    message("Thank you for taking your time to read this.")
    message("===================================================================")
    set (WL_COMPILERFLAG_OLDSTYLECAST " ${PARAMETER_COMPILERFLAG_OLDSTYLECAST}") #the space is on purpose!
  ELSE (WLBUILD_COMPILERVERSION_REP STREQUAL "4.3.2" OR WLBUILD_COMPILERVERSION_REP STREQUAL "4.3.1")
    IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
      set (WL_COMPILERFLAG_OLDSTYLECAST " ${PARAMETER_COMPILERFLAG_OLDSTYLECAST_EXTENDED}") #the space is on purpose!
    ELSE (CMAKE_BUILD_TYPE STREQUAL "Debug")
      include_directories(SYSTEM ${Boost_INCLUDE_DIR})
    ENDIF (CMAKE_BUILD_TYPE STREQUAL "Debug")
  ENDIF (WLBUILD_COMPILERVERSION_REP STREQUAL "4.3.2" OR WLBUILD_COMPILERVERSION_REP STREQUAL "4.3.1")
ENDIF (Compiler_old-style-cast_supported)

CHECK_CXX_COMPILER_FLAG(${PARAMETER_COMPILERFLAG_GENERICWARNINGS} Compiler_generic_warnings_supported)
IF (Compiler_generic_warnings_supported)
  set (WL_COMPILERFLAG_GENERICWARNINGS " ${PARAMETER_COMPILERFLAG_GENERICWARNINGS}") #the space is on purpose!
ENDIF (Compiler_generic_warnings_supported)

CHECK_CXX_COMPILER_FLAG(${PARAMETER_COMPILERFLAG_EXTRAWARNINGS} Compiler_extra_warnings_supported)
IF (Compiler_extra_warnings_supported)
  set (WL_COMPILERFLAG_EXTRAWARNINGS " ${PARAMETER_COMPILERFLAG_EXTRAWARNINGS}") #the space is on purpose!
ELSE (Compiler_extra_warnings_supported)
  message("Warning: couldn't set the following compiler options: ${PARAMETER_COMPILERFLAG_EXTRAWARNINGS}. Most likely these options are available in a newer release of your compiler.")
ENDIF (Compiler_extra_warnings_supported)

CHECK_CXX_COMPILER_FLAG(${PARAMETER_COMPILERFLAG_GCCWARNINGS} Compiler_gcc_warnings_supported)
IF (Compiler_gcc_warnings_supported)
  set (WL_COMPILERFLAG_GCCWARNINGS " ${PARAMETER_COMPILERFLAG_GCCWARNINGS}") #the space is on purpose!
ELSE (Compiler_gcc_warnings_supported)
  message("Warning: could not add additional GCC-specific warning options: ${PARAMETER_COMPILERFLAG_GCCWARNINGS}. Most likely you are using a different compiler, like Clang/LLVM or Visual Studio.")
ENDIF (Compiler_gcc_warnings_supported)

CHECK_CXX_COMPILER_FLAG(${PARAMETER_COMPILERFLAG_GCCWARNINGS} Compiler_gcc_warnings_supported)

IF (WL_STRICT)
  CHECK_CXX_COMPILER_FLAG(${PARAMETER_COMPILERFLAG_STRICT} Compiler_strict_mode_supported)
  IF (Compiler_strict_mode_supported)
    set (WL_COMPILERFLAG_STRICT " ${PARAMETER_COMPILERFLAG_STRICT}") #the space is on purpose!
    message(STATUS "Strict mode enabled, expect compilation errors instead of compilation warnings!")
  ENDIF (Compiler_strict_mode_supported)
ENDIF (WL_STRICT)

# CMAKE only defines "-g", but we need -DDEBUG also, and we need -DNOPARACHUTE (for SDL) in Debug
set (CMAKE_CXX_FLAGS_DEBUG "${WL_COMPILERFLAG_CXXSTD} -g -DDEBUG -DNOPARACHUTE${WL_COMPILERFLAG_OLDSTYLECAST}${WL_COMPILERFLAG_GENERICWARNINGS}${WL_COMPILERFLAG_EXTRAWARNINGS}${WL_COMPILERFLAG_GCCWARNINGS}${WL_COMPILERFLAG_STRICT}" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)

set (WL_COMPILERFLAG_OPTIMIZATIONS "-O3")

#This can be removed if no one uses gcc 4.5.1 or 4.5.2 any more
IF (CMAKE_COMPILER_IS_GNUCXX)
  EXEC_PROGRAM (
    ${CMAKE_CXX_COMPILER}
    ARGS --version
    OUTPUT_VARIABLE WLBUILD_COMPILERVERSION
  )
  STRING(REGEX REPLACE ".*(4)\\.(5)\\.([0-9]).*" "\\1.\\2.\\3" WLBUILD_COMPILERVERSION_REP ${WLBUILD_COMPILERVERSION})
  IF (WLBUILD_COMPILERVERSION_REP STREQUAL "4.5.1" OR WLBUILD_COMPILERVERSION_REP STREQUAL "4.5.2")
    message("Detected gcc ${WLBUILD_COMPILERVERSION_REP}")
    message("Suffering from gcc bug, disabling -O3")
    set (WL_COMPILERFLAG_OPTIMIZATIONS "-O2")
  ENDIF (WLBUILD_COMPILERVERSION_REP STREQUAL "4.5.1" OR WLBUILD_COMPILERVERSION_REP STREQUAL "4.5.2")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

set (CMAKE_CXX_FLAGS_RELEASE "${WL_COMPILERFLAG_CXXSTD} ${WL_COMPILERFLAG_OPTIMIZATIONS} -DNDEBUG${WL_COMPILERFLAG_OLDSTYLECAST}${WL_COMPILERFLAG_GENERICWARNINGS}${WL_COMPILERFLAG_EXTRAWARNINGS}${WL_COMPILERFLAG_GCCWARNINGS}${WL_COMPILERFLAG_STRICT}" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)

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
  add_definitions( -DREDIRECT_OUTPUT )

  # Automatic linking for boost requires setting lib dir and clearing Boost_LIBRARIES
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
  set(Boost_LIBRARIES "")
  include_directories(${Boost_INCLUDE_DIR})


  find_library(INTL_LIBRARY NAMES intl libintl)

  set (MSVC_DISABLE_WARNINGS "/wd4146 /wd4290 /wd4355 /wd4373 /wd4800")
  # CMAKE only defines "-g", but we need -DDEBUG also, and we need -DNOPARACHUTE (for SDL) in Debug
  set (CMAKE_CXX_FLAGS_DEBUG " -DDEBUG -DNOPARACHUTE /MTd /Zi /D_DEBUG ${MSVC_DISABLE_WARNINGS}" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)

  set (CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG /MT /GL ${MSVC_DISABLE_WARNINGS}" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)

  set (CMAKE_EXE_LINKER_FLAGS_DEBUG "/debug /LTCG /NODEFAULTLIB:libcmt.lib" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)
endif (DEFINED MSVC)

IF (MINGW)
  set (CMAKE_EXE_LINKER_FLAGS "-Wl,--large-address-aware" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)
  message (STATUS "Enabled large address awareness on mingw32")
endif (MINGW)

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message (STATUS "Using compiler flags: ${CMAKE_CXX_FLAGS_DEBUG}")
ELSE (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message (STATUS "Using compiler flags: ${CMAKE_CXX_FLAGS_RELEASE}")
ENDIF (CMAKE_BUILD_TYPE STREQUAL "Debug")

if (DEFINED WL_3RDPARTY_DIR)
  set (CMAKE_INCLUDE_PATH "${WL_3RDPARTY_DIR}/include;${WL_3RDPARTY_DIR}/include/sdl")
  set (CMAKE_LIBRARY_PATH "${WL_3RDPARTY_DIR}/lib")
endif (DEFINED WL_3RDPARTY_DIR)

# on BSD this must be explicitly linked
if (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
  # Not needed on Debian GNU/kFreeBSD..
  if (NOT CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")
    find_library(EXECINFO_LIBRARY NAMES execinfo)
  endif (NOT CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")
endif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")


# Check for opengl
# TODO Check for SDL_opengl.h and add to include path
find_package(OpenGL REQUIRED)
# OpenGL Headers are not needed directly. Instead SDL_opengl.h should be searched
find_package(GLEW REQUIRED)
add_definitions(${GLEW_EXTRA_DEFINITIONS})

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
	# Needed - else detect_revision.py will still use bzr or even crash, if
	# Widelands is compiled in a bzr export
	file (WRITE WL_RELEASE "${WL_VERSION}")
endif (DEFINED WL_VERSION_MAJOR)

# we need python to determine the current version
find_package (PythonInterp REQUIRED)

if (NOT DEFINED WL_VERSION)
  add_custom_target (
    BzrRevision ALL
    COMMAND ${CMAKE_COMMAND} -DWL_INSTALL_PREFIX=${WL_INSTALL_PREFIX} -DWL_INSTALL_BINDIR=${WL_INSTALL_BINDIR} -DWL_INSTALL_DATADIR=${WL_INSTALL_DATADIR} -DWL_INSTALL_LOCALEDIR=${WL_INSTALL_LOCALEDIR} -DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/BzrRevision.cmake
  )

  # Detect version now
  execute_process (
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/utils/detect_revision.py
    OUTPUT_VARIABLE WL_VERSION
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/VERSION "${WL_VERSION}")
  configure_file (${CMAKE_CURRENT_SOURCE_DIR}/src/build_info.cc.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/build_info.cc)
  message (STATUS "Version of Widelands Build is ${WL_VERSION}(${CMAKE_BUILD_TYPE})")

else (NOT DEFINED WL_VERSION)
  add_custom_target (
    InputRevision ALL
    COMMAND ${CMAKE_COMMAND} -DWL_INSTALL_PREFIX=${WL_INSTALL_PREFIX} -DWL_INSTALL_BINDIR=${WL_INSTALL_BINDIR} -DWL_INSTALL_DATADIR=${WL_INSTALL_DATADIR} -DWL_INSTALL_LOCALEDIR=${WL_INSTALL_LOCALEDIR} -DWL_VERSION=${WL_VERSION} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/InputRevision.cmake
  )
endif (NOT DEFINED WL_VERSION)

install (
  FILES ${CMAKE_CURRENT_BINARY_DIR}/VERSION
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT CoreVersionFile
)
set(CPACK_COMPONENT_COREVERSIONFILE_HIDDEN TRUE)
set(CPACK_COMPONENT_COREVERSIONFILE_REQUIRED TRUE)

include_directories(src ${CMAKE_CURRENT_BINARY_DIR}/src)

include(CheckIncludeFile)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})

# Check if we have a working minizip library
find_package(Minizip)
if (MINIZIP_FOUND)
  include_directories(${MINIZIP_INCLUDE_DIR})
endif (MINIZIP_FOUND)

if (APPLE OR WIN32 OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  if (NOT CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")
    find_package(intl REQUIRED)
    include_directories(${INTL_INCLUDE_DIR})
    endif (NOT CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")
endif (APPLE OR WIN32 OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")

# Gettext is required, but it does not provide GETTEXT_INCLUDE_DIR or
# GETTEXT_LIBRARY as "usual" libraries do
find_package(Gettext REQUIRED)

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

find_package(SDL_image REQUIRED)
include_directories(${SDLIMAGE_INCLUDE_DIR})
if (NOT SDLIMAGE_FOUND)
  message(FATAL_ERROR "Required package SDL_image not found, please install and re-run cmake")
endif (NOT SDLIMAGE_FOUND)

find_package(SDL_mixer REQUIRED)
include_directories(${SDLMIXER_INCLUDE_DIR})
if (NOT SDLMIXER_FOUND)
  message(FATAL_ERROR "Required package SDL_mixer not found, please install and re-run cmake")
endif (NOT SDLMIXER_FOUND)

find_package(SDL_net REQUIRED)
include_directories(${SDLNET_INCLUDE_DIR})
if (NOT SDLNET_FOUND)
  message(FATAL_ERROR "Required package SDL_net not found, please install and re-run cmake")
endif (NOT SDLNET_FOUND)

find_package(SDL_ttf REQUIRED)
include_directories(${SDLTTF_INCLUDE_DIR})
if (NOT SDLTTF_FOUND)
  message(FATAL_ERROR "Required package SDL_ttf not found, please install and re-run cmake")
endif (NOT SDLTTF_FOUND)

find_package(SDL_gfx REQUIRED)
include_directories(${SDLGFX_INCLUDE_DIR})
if (NOT SDLGFX_FOUND)
  message(FATAL_ERROR "Required package SDL_gfx not found, please install and re-run cmake")
endif (NOT SDLGFX_FOUND)

IF (WIN32)
  SET(GUI_TYPE WIN32)
ENDIF (WIN32)

#configure_file (src/config.h.cmake src/config.h)
#configure_file (src/build_info.cc.cmake src/build_info.cc)

add_subdirectory(doc)
add_subdirectory(po)
add_subdirectory(src)

# install files to the correct locations here
install(
  DIRECTORY
    campaigns
    fonts
    global
    tribes
    txts
    worlds
    pics
    scripting
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT CoreDataFiles
)
set(CPACK_COMPONENT_COREDATAFILES_DISPLAY_NAME "Widelands Core")
set(CPACK_COMPONENT_COREDATAFILES_HIDDEN FALSE)
set(CPACK_COMPONENT_COREDATAFILES_REQUIRED TRUE)

install(
  DIRECTORY
    maps
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT MapFiles
)
set(CPACK_COMPONENT_MAPFILES_DISPLAY_NAME "Maps Files")

install(
  DIRECTORY
    music
	sound
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT MusicFiles
)
set(CPACK_COMPONENT_MUSICFILES_DISPLAY_NAME "Music and Sound Files")

install(
  FILES
    COPYING
    CREDITS
    ChangeLog
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT CoreLicenseFiles
)
set(CPACK_COMPONENT_CORELICENSEFILES_HIDDEN TRUE)
set(CPACK_COMPONENT_CORELICENSEFILES_REQUIRED TRUE)

install(
  DIRECTORY
    doc
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug
  COMPONENT DocFiles
  PATTERN "CMakeLists.txt" EXCLUDE
)
set(CPACK_COMPONENT_DOCFILES_DISPLAY_NAME "Documentation")
set(CPACK_COMPONENT_DOCFILES_HIDDEN TRUE)
set(CPACK_COMPONENT_DOCFILES_REQUIRED FALSE)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES locale)

install(
  DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/locale/
  DESTINATION ${WL_INSTALL_DATADIR}/locale
  CONFIGURATIONS Debug;Release
  COMPONENT CoreLanguageFiles
)
set(CPACK_COMPONENT_CORELANGUAGEFILES_DISPLAY_NAME "Language Files")

set(CPACK_COMPONENT_EXECUTABLEFILES_HIDDEN TRUE)
set(CPACK_COMPONENT_EXECUTABLEFILES_REQUIRED TRUE)

# Components to install
set(CPACK_COMPONENTS_ALL ExecutableFiles CoreDataFiles MapFiles CoreLanguageFiles CoreVersionFile CoreLicenseFiles)
if (NOT WL_NOMUSIC)
  set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} MusicFiles)
endif (NOT WL_NOMUSIC)

if (WIN32)
  # Install files only needed for Windows
  FILE(GLOB dlls "${CMAKE_CURRENT_BINARY_DIR}/*.dll")
  install(
    FILES
      ${dlls}
      utils/win32/WL-Editor.ico
    DESTINATION ${WL_INSTALL_DATADIR}
    CONFIGURATIONS Debug;Release
    COMPONENT Win32Specific
  )
  set(CPACK_COMPONENT_WIN32SPECIFIC_HIDDEN TRUE)
  set(CPACK_COMPONENT_WIN32SPECIFIC_REQUIRED TRUE)
  set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} Win32Specific)

  # Only NSIS supports install types
  set(CPACK_ALL_INSTALL_TYPES Full Minimal)
  set(CPACK_COMPONENT_EXECUTABLEFILES_INSTALL_TYPES Full Minimal)
  set(CPACK_COMPONENT_COREDATAFILES_INSTALL_TYPES Full Minimal)
  set(CPACK_COMPONENT_COREVERSIONFILE_INSTALL_TYPES Full Minimal)
  set(CPACK_COMPONENT_CORELICENSEFILES_INSTALL_TYPES Full Minimal)
  set(CPACK_COMPONENT_MAPFILES_INSTALL_TYPES Full)
  set(CPACK_COMPONENT_MUSICFILES_INSTALL_TYPES Full)
  set(CPACK_COMPONENT_CORELANGUAGEFILES_INSTALL_TYPES Full)
  set(CPACK_COMPONENT_WIN32SPECIFIC_INSTALL_TYPES Full Minimal)

  # Set generator
  set(CPACK_GENERATOR NSIS)

  # Graphics for the installer (all slashes are necessary!)
  set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_LIST_DIR}\\\\utils\\\\win32\\\\wl-ico-128.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_LIST_DIR}\\\\utils\\\\win32\\\\wl-ico-128.ico")
  set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_LIST_DIR}\\\\utils\\\\win32\\\\nsis\\\\install_header.bmp")
  set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CMAKE_CURRENT_LIST_DIR}\\\\utils\\\\win32\\\\nsis\\\\WL.bmp")

  # Name for installer and Windows Add/Remove
  set(CPACK_NSIS_PACKAGE_NAME "Widelands")
  set(CPACK_NSIS_DISPLAY_NAME "Widelands ${WL_VERSION}")
  set(CPACK_NSIS_DISPLAY_NAME_SET "TRUE")

  # Optional dialogs
  if (NOT DEFINED CPACK_NSIS_LANGUAGE_DIALOG)
    set(CPACK_NSIS_LANGUAGE_DIALOG "ON")
  endif (NOT DEFINED CPACK_NSIS_LANGUAGE_DIALOG)
  if (NOT DEFINED CPACK_NSIS_CHANGELOG)
    set(CPACK_NSIS_CHANGELOG "ON")
  endif (NOT DEFINED CPACK_NSIS_CHANGELOG)

  # Setup (and remove) shortcuts to Widelands (all slashes are necessary!)
  set(CPACK_PACKAGE_EXECUTABLES "..\\\\widelands;Widelands")
  set(CPACK_NSIS_MENU_LINKS "http://wl.widelands.org/;Widelands Homepage")
  set(CPACK_NSIS_CREATE_ICONS_EXTRA "
    CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Widelands - Mapeditor.lnk\\\" \\\"$INSTDIR\\\\widelands.exe\\\" \\\"--editor\\\" \\\"$INSTDIR\\\\WL-Editor.ico\\\"
  ")
  set(CPACK_NSIS_DELETE_ICONS_EXTRA "
    Delete \\\"$DESKTOP\\\\Widelands.lnk\\\"
    Delete \\\"$DESKTOP\\\\Widelands - Mapeditor.lnk\\\"
    Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Widelands - Mapeditor.lnk\\\"
  ")

  # Additional information (all slashes are necessary!)
  set(CPACK_NSIS_URL_INFO_ABOUT "http://wl.widelands.org/")
  set(CPACK_NSIS_HELP_LINK "http://wl.widelands.org/")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "widelands.exe")
  set(CPACK_NSIS_MUI_FINISHPAGE_RUN "..\\\\widelands.exe")
  set(CPACK_NSIS_MUI_FINISHPAGE_README "$INSTDIR\\\\ChangeLog")
  set(CPACK_PACKAGE_VENDOR "Widelands Development Team")
  # replace [ and ], they will cause problems in filename!
  string(REPLACE "[" "-" PACKAGE_VERSION ${WL_VERSION})
  string(REPLACE "]" "" PACKAGE_VERSION ${PACKAGE_VERSION})
  set(CPACK_PACKAGE_VERSION "${PACKAGE_VERSION}(${CMAKE_BUILD_TYPE})")
  set(CPACK_PACKAGE_VERSION_MAJOR ${WL_VERSION_MAJOR})
  set(CPACK_PACKAGE_VERSION_MINOR ${WL_VERSION_MINOR})
  set(CPACK_PACKAGE_VERSION_PATCH ${WL_VERSION_PATCH})
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_LIST_DIR}\\\\COPYING")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_LIST_DIR}\\\\ChangeLog")

  # Setup (and remove) Registry entries for Widelands (all slashes are necessary!)
  set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
    WriteRegStr HKCR \\\".wgf\\\" \\\"\\\" \\\"WidelandsSavegame\\\"
    WriteRegStr HKCR \\\"WidelandsSavegame\\\" \\\"\\\" \\\"Widelands Savegame\\\"
    WriteRegStr HKCR \\\"WidelandsSavegame\\\\DefaultIcon\\\" \\\"\\\" \\\"$INSTDIR\\\\widelands.exe,0\\\"
    WriteRegStr HKCR \\\"WidelandsSavegame\\\\shell\\\\open\\\\command\\\" \\\"\\\" \\\"$INSTDIR\\\\widelands.exe --loadgame=%1\\\"

    WriteRegStr HKCR \\\".wmf\\\" \\\"\\\" \\\"WidelandsMapFile\\\"
    WriteRegStr HKCR \\\"WidelandsMapFile\\\" \\\"\\\" \\\"Widelands Mapfile\\\"
    WriteRegStr HKCR \\\"WidelandsMapFile\\\\DefaultIcon\\\" \\\"\\\" \\\"$INSTDIR\\\\WL-Editor.ico\\\"
    WriteRegStr HKCR \\\"WidelandsMapFile\\\\shell\\\\open\\\\command\\\" \\\"\\\" \\\"$INSTDIR\\\\widelands.exe --editor=%1\\\"
  ")
  set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
    DeleteRegValue HKCR \\\".wgf\\\" \\\"WidelandsSavegame\\\"
    DeleteRegKey /ifempty HKCR \\\".wgf\\\"
    DeleteRegKey HKCR \\\"WidelandsSavegame\\\"

    DeleteRegValue HKCR \\\".wmf\\\" \\\"WidelandsMapFile\\\"
    DeleteRegKey /ifempty HKCR \\\".wmf\\\"
    DeleteRegKey HKCR \\\"WidelandsMapFile\\\"

    Delete \\\"$INSTDIR\\\\stdout.txt\\\"
    Delete \\\"$INSTDIR\\\\stderr.txt\\\"
    Delete \\\"$INSTDIR\\\"
  ")

elseif (WIN32)
  set (CPACK_PACKAGE_FILE_NAME widelands-${WL_VERSION})
  set (CPACK_GENERATOR "TGZ")
  set (CPACK_SOURCE_GENERATOR "TGZ")
endif (WIN32)

include(CPack)
