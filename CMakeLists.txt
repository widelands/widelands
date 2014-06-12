project (widelands)

cmake_minimum_required (VERSION 2.8)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Build directory and source directory must not be the same.")
endif (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

# Define the directory structure for installation - will be hardcoded in WL bin
# (through config.h).
macro(_set_if_unset VAR VALUE)
  if (NOT ${VAR} OR ${VAR} STREQUAL "")
    set (${VAR} ".")
  endif()
endmacro(_set_if_unset)

if(WIN32 OR APPLE OR WL_PORTABLE)
  _set_if_unset(WL_INSTALL_PREFIX ".")
  _set_if_unset(WL_INSTALL_DATADIR ".")
  _set_if_unset(WL_INSTALL_LOCALEDIR "locale")
  _set_if_unset(WL_INSTALL_BINDIR ".")
else()
  _set_if_unset(WL_INSTALL_PREFIX "/usr/local")
  _set_if_unset(WL_INSTALL_DATADIR "share/games/widelands")
  _set_if_unset(WL_INSTALL_LOCALEDIR "${WL_INSTALL_PREFIX}/${WL_INSTALL_DATADIR}/locale")
  _set_if_unset(WL_INSTALL_BINDIR "games")
endif()

if (CMAKE_INSTALL_PREFIX STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Build directory and install directory must not be the same.")
endif (CMAKE_INSTALL_PREFIX STREQUAL CMAKE_BINARY_DIR)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
    message(FATAL_ERROR "Widelands needs GCC >= 4.7 to compile.")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.2)
    message(FATAL_ERROR "Clang version must be at least 3.2!")
  endif()
else()
  message(WARNING "You are using an unsupported compiler! Supported are Clang and GCC.")
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  find_path(FILE_WL_RELEASE "WL_RELEASE" ${CMAKE_CURRENT_SOURCE_DIR})
  if(${FILE_WL_RELEASE} STREQUAL "FILE_WL_RELEASE-NOTFOUND")
    set(CMAKE_BUILD_TYPE Debug)
  else()
    set(CMAKE_BUILD_TYPE Release)
  endif()
endif (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")

set (CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})

# we only include Boost Headers to the main executable, no libraries
# unit_test_framework is for testing only
message(STATUS "Enabled unit tests")
set (Boost_USE_STATIC_LIBS  OFF)
set (Boost_FIND_COMPONENTS unit_test_framework)
set (Boost_USE_MULTITHREADED ON)
set (Boost_DETAILED_FAILURE_MSG ON)
find_package(Boost 1.48
  COMPONENTS
    unit_test_framework
    regex
  REQUIRED)

# If boost unit test library is linked dynamically, BOOST_TEST_DYN_LINK must be defined
string( REGEX MATCH ".a$" BOOST_STATIC_UNIT_TEST_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
if (NOT BOOST_STATIC_UNIT_TEST_LIB)
  add_definitions(-DBOOST_TEST_DYN_LINK)
endif (NOT BOOST_STATIC_UNIT_TEST_LIB)

include(CTest)
enable_testing()

# Run a test after a normal compile. This magic is needed as 'make test' will
# not rebuild tests:
# http://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
add_custom_target(_run_all_tests ALL
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS wl_tests
)

#TODO: (code review):
#And the last additional option passed to the compiler seem to have been
#reversed and enabled by default, so it shouldn't be necessary. Compare
#https://gcc.gnu.org/onlinedocs/gcc-4.5.0/gcc/Language-Independent-Options.html
#https://gcc.gnu.org/onlinedocs/gcc-4.6.0/gcc/Language-Independent-Options.html

set (PARAMETER_COMPILERFLAG_BOOST_INCLUDE_DIR "-isystem ${Boost_INCLUDE_DIR}")
set (PARAMETER_COMPILERFLAG_GENERICWARNINGS "-Wall -Wextra")
set (PARAMETER_COMPILERFLAG_EXTRAWARNINGS "-Wformat -Wformat-nonliteral -Wformat-security -Wformat-y2k -Winit-self -Winvalid-pch -Wmissing-include-dirs -Wold-style-cast -Woverlength-strings -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-promo -Wswitch-default -Wundef -Wunused -Wunused-macros")
set (PARAMETER_COMPILERFLAG_GCCWARNINGS "-Wlogical-op -Wsync-nand -Wtrampolines")
set (PARAMETER_COMPILERFLAG_STRICT "-Werror -Wno-error=old-style-cast -Wno-error=deprecated-declarations -fdiagnostics-show-option")
include(CheckCXXCompilerFlag) #this include should be safe

# Enable c++11 features.
if (WIN32)
  set (PARAMETER_COMPILERFLAG_CXXSTD "-std=gnu++11")
else()
  set (PARAMETER_COMPILERFLAG_CXXSTD "-std=c++11")
endif()

CHECK_CXX_COMPILER_FLAG(${PARAMETER_COMPILERFLAG_CXXSTD} Compiler_std_supported)
IF (Compiler_std_supported)
  set (WL_COMPILERFLAG_CXXSTD " ${PARAMETER_COMPILERFLAG_CXXSTD}") #the space is on purpose!
ENDIF (Compiler_std_supported)

CHECK_CXX_COMPILER_FLAG(${PARAMETER_COMPILERFLAG_BOOST_INCLUDE_DIR} Compiler_boost_include_dir_supported)
IF (Compiler_boost_include_dir_supported)
  IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (WL_COMPILERFLAG_BOOST_INCLUDE_DIR " ${PARAMETER_COMPILERFLAG_BOOST_INCLUDE_DIR}") #the space is on purpose!
  ELSE (CMAKE_BUILD_TYPE STREQUAL "Debug")
    include_directories(SYSTEM ${Boost_INCLUDE_DIR})
  ENDIF (CMAKE_BUILD_TYPE STREQUAL "Debug")
ENDIF()

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

# CMAKE only defines "-g", but we need -DDEBUG also, and we need -DNOPARACHUTE (for SDL) in Debug
set (CMAKE_CXX_FLAGS_DEBUG "${WL_COMPILERFLAG_CXXSTD} -g -DDEBUG -DNOPARACHUTE${WL_COMPILERFLAG_BOOST_INCLUDE_DIR}${WL_COMPILERFLAG_GENERICWARNINGS}${WL_COMPILERFLAG_EXTRAWARNINGS}${WL_COMPILERFLAG_GCCWARNINGS}${WL_COMPILERFLAG_STRICT}" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)

set (WL_COMPILERFLAG_OPTIMIZATIONS "-O3")

set (CMAKE_CXX_FLAGS_RELEASE "${WL_COMPILERFLAG_CXXSTD} ${WL_COMPILERFLAG_OPTIMIZATIONS} -DNDEBUG${WL_COMPILERFLAG_GENERICWARNINGS}${WL_COMPILERFLAG_EXTRAWARNINGS}${WL_COMPILERFLAG_GCCWARNINGS}${WL_COMPILERFLAG_STRICT}" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)

IF (WIN32)
  set (CMAKE_EXE_LINKER_FLAGS "-Wl,--large-address-aware" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)
  message (STATUS "Enabled large address awareness on mingw32")
endif (WIN32)

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

find_package (PythonInterp REQUIRED)

# Check for opengl
# TODO Check for SDL_opengl.h and add to include path
find_package(OpenGL REQUIRED)
# OpenGL Headers are not needed directly. Instead SDL_opengl.h should be searched
find_package(GLEW REQUIRED)
add_definitions(${GLEW_EXTRA_DEFINITIONS})

# ...but using MAJOR, MINOR and PATCH
if (DEFINED WL_VERSION_MAJOR)

	# Until 1.x.x set standard VERSION => "BuildXX"
	if (WL_VERSION_MAJOR STREQUAL "0")
		if (DEFINED WL_VERSION_MINOR)
			set (WL_VERSION Build${WL_VERSION_MINOR})
			if (DEFINED WL_VERSION_PATCH)
				set (WL_VERSION ${WL_VERSION}.${WL_VERSION_PATCH})
			else (DEFINED WL_VERSION_PATCH)
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
			if (DEFINED WL_VERSION_PATCH)
				set (WL_VERSION ${WL_VERSION}.${WL_VERSION_PATCH})
			else (DEFINED WL_VERSION_PATCH)
			endif (DEFINED WL_VERSION_PATCH)
			if (DEFINED WL_VERSION_ADDITION)
				set (WL_VERSION ${WL_VERSION}${WL_VERSION_ADDITION})
			endif (DEFINED WL_VERSION_ADDITION)
		else (DEFINED WL_VERSION_MINOR)
		endif (DEFINED WL_VERSION_MINOR)
	endif (WL_VERSION_MAJOR STREQUAL "0")
	# Needed - else detect_revision.py will still use bzr or even crash, if
	# Widelands is compiled in a bzr export
	file (WRITE WL_RELEASE "${WL_VERSION}")
endif (DEFINED WL_VERSION_MAJOR)

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

include_directories(src ${CMAKE_CURRENT_BINARY_DIR}/src)

include(CheckIncludeFile)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})

if (APPLE OR WIN32 OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  if (NOT CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")
    find_package(intl REQUIRED)
    include_directories(${INTL_INCLUDE_DIR})
    endif (NOT CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")
endif (APPLE OR WIN32 OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")

# Gettext is required, but it does not provide GETTEXT_INCLUDE_DIR or
# GETTEXT_LIBRARY as "usual" libraries do
find_package(Gettext REQUIRED)

find_package(ZLIB REQUIRED)

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

install(
  DIRECTORY
    maps
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT MapFiles
)

install(
  DIRECTORY
    music
	sound
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT MusicFiles
)

install(
  FILES
    COPYING
    CREDITS
    ChangeLog
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT CoreLicenseFiles
)

install(
  DIRECTORY
    doc
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug
  COMPONENT DocFiles
  PATTERN "CMakeLists.txt" EXCLUDE
)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES locale)

install(
  DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/locale/
  DESTINATION ${WL_INSTALL_DATADIR}/locale
  CONFIGURATIONS Debug;Release
  COMPONENT CoreLanguageFiles
)
