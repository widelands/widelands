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

set (Boost_USE_STATIC_LIBS  OFF)
set (Boost_USE_MULTITHREADED ON)
set (Boost_DETAILED_FAILURE_MSG ON)
find_package(Boost 1.48
  COMPONENTS
    unit_test_framework
    regex
  REQUIRED)

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

include(CheckCXXCompilerFlag)
function(_add_flag_if_supported VAR_NAME FLAG TEST_NAME)
  CHECK_CXX_COMPILER_FLAG(${FLAG} ${TEST_NAME})
  if(${TEST_NAME})
    if (${VAR_NAME})
      set(${VAR_NAME} "${${VAR_NAME}} ${FLAG}" PARENT_SCOPE)
    else()
      set(${VAR_NAME} "${FLAG}" PARENT_SCOPE)
    endif()
  endif()
endfunction()

_add_flag_if_supported(WL_GENERIC_CXX_FLAGS "-std=c++11" CXX_SUPPORTS_STD_CPP11)
_add_flag_if_supported(WL_GENERIC_CXX_FLAGS "-std=gnu++11" CXX_SUPPORTS_STD_CPP11_GNU)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(WL_DEBUG_FLAGS "-g -DDEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(WL_OPTIMIZE_FLAGS "-O3")
  set(WL_DEBUG_FLAGS "-DNDEBUG -DNOPARACHUTE")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set(WL_OPTIMIZE_FLAGS "-O3")
  set(WL_DEBUG_FLAGS "-g -DNDEBUG -DNOPARACHUTE")
else()
  message(FATAL_ERROR "Unknown CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

# Enable warnings.
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wall" CXX_SUPPORTS_WALL)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Weverything" CXX_SUPPORTS_WEVERYTHING)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wpedantic" CXX_SUPPORTS_PEDANTIC)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wdeprecated-declarations" CXX_SUPPORTS_WDEPRECATED_DECLARATIONS)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wextra" CXX_SUPPORTS_WEXTRA)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wformat" CXX_SUPPORTS_WFORMAT)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wformat-nonliteral" CXX_SUPPORTS_WFORMAT_NONLITERAL)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wformat-security" CXX_SUPPORTS_WFORMAT_SECURITY)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wformat-y2k" CXX_SUPPORTS_WFORMAT_Y2K)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Winit-self" CXX_SUPPORTS_WINIT_SELF)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Winvalid-pch" CXX_SUPPORTS_WINVALID_PCH)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wlogical-op" CXX_SUPPORTS_WLOGICAL_OP)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wmissing-include-dirs" CXX_SUPPORTS_WMISSING_INCLUDE_DIRS)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wno-undef" CXX_SUPPORTS_WNO_UNDEF)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wold-style-cast" CXX_SUPPORTS_WOLD_STYLE_CAST)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Woverlength-strings" CXX_SUPPORTS_WOVERLENGTH_STRINGS)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wpacked" CXX_SUPPORTS_WPACKED)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wpointer-arith" CXX_SUPPORTS_WPOINTER_ARITH)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wredundant-decls" CXX_SUPPORTS_WREDUNDANT_DECLS)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wshadow" CXX_SUPPORTS_WSHADOW)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wsign-promo" CXX_SUPPORTS_WSIGN_PROMO)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wswitch-default" CXX_SUPPORTS_WSWITCH_DEFAULT)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wsync-nand" CXX_SUPPORTS_WSYNC_NAND)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wtrampolines" CXX_SUPPORTS_WTRAMPOLINES)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wundef" CXX_SUPPORTS_WUNDEF)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wunused" CXX_SUPPORTS_WUNUSED)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wunused-macros" CXX_SUPPORTS_WUNUSED_MACROS)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-fdiagnostics-show-option" CXX_SUPPORTS_FDIAGNOSTICS_SHOW_OPTION)

# Turn some warnings into errors.
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Werror=uninitialized" CXX_SUPPORTS_WERROR_UNINITIALIZED)

# Disabled warnings that are overly verbose right now or just do not make sense.
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wno-padded" CXX_SUPPORTS_WNO_PADDED)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wno-c++98-compat" CXX_SUPPORTS_WNO_CPP98_COMPAT)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wno-c++98-compat-pedantic" CXX_SUPPORTS_WNO_CPP98_COMPAT_PEDANTIC)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wno-sign-conversion" CXX_SUPPORTS_WNO_SIGN_CONVERSION)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wno-conversion" CXX_SUPPORTS_WNO_CONVERSION)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wno-exit-time-destructors" CXX_SUPPORTS_WNO_EXIT_TIME_DESTRUCTOR)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wno-global-constructors" CXX_SUPPORTS_WNO_GLOBAL_CONSTRUCTORS)
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Qunused-arguments" CXX_SUPPORTS_QUNUSED_ARGUMENTS)


# TODO(sirver): weak-vtables should be enabled, but leads to lot of errors right now.
_add_flag_if_supported(WL_COMPILE_DIAGNOSTICS "-Wno-weak-vtables" CXX_SUPPORTS_WNO_WEAK_VTABLES)

IF (WIN32)
  set (CMAKE_EXE_LINKER_FLAGS "-Wl,--large-address-aware" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)
  message (STATUS "Enabled large address awareness on mingw32")
endif (WIN32)

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

if (NOT DEFINED WL_VERSION)
  add_custom_target (
    BzrRevision ALL
    COMMAND ${CMAKE_COMMAND} -DWL_INSTALL_PREFIX=${WL_INSTALL_PREFIX} -DWL_INSTALL_BINDIR=${WL_INSTALL_BINDIR} -DWL_INSTALL_DATADIR=${WL_INSTALL_DATADIR} -DWL_INSTALL_LOCALEDIR=${WL_INSTALL_LOCALEDIR} -DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/BzrRevision.cmake
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
    COMMAND ${CMAKE_COMMAND} -DWL_INSTALL_PREFIX=${WL_INSTALL_PREFIX} -DWL_INSTALL_BINDIR=${WL_INSTALL_BINDIR} -DWL_INSTALL_DATADIR=${WL_INSTALL_DATADIR} -DWL_INSTALL_LOCALEDIR=${WL_INSTALL_LOCALEDIR} -DWL_VERSION=${WL_VERSION} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/InputRevision.cmake
  )
endif (NOT DEFINED WL_VERSION)

install (
  FILES ${CMAKE_CURRENT_BINARY_DIR}/VERSION
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT CoreVersionFile
)

include(CheckIncludeFile)

find_package(PNG REQUIRED)
# TODO(sirver): Do not globally include for all targets.
include_directories(${PNG_INCLUDE_DIR})

find_package(ZLIB REQUIRED)
# TODO(sirver): Do not globally include for all targets.
include_directories(${ZLIB_INCLUDE_DIR})

if (APPLE OR WIN32 OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  if (NOT CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")
    find_package(intl REQUIRED)
    # TODO(sirver): Do not globally include for all targets.
    include_directories(${INTL_INCLUDE_DIR})
    endif (NOT CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")
endif (APPLE OR WIN32 OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")

# Gettext is required, but it does not provide GETTEXT_INCLUDE_DIR or
# GETTEXT_LIBRARY as "usual" libraries do
find_package(Gettext REQUIRED)

find_package(ZLIB REQUIRED)

find_package(SDL REQUIRED)
# TODO(sirver): Do not globally include for all targets.
include_directories(${SDL_INCLUDE_DIR})

find_package(SDL_image REQUIRED)
# TODO(sirver): Do not globally include for all targets.
include_directories(${SDLIMAGE_INCLUDE_DIR})

find_package(SDL_mixer REQUIRED)
# TODO(sirver): Do not globally include for all targets.
include_directories(${SDLMIXER_INCLUDE_DIR})

find_package(SDL_net REQUIRED)
# TODO(sirver): Do not globally include for all targets.
include_directories(${SDLNET_INCLUDE_DIR})

find_package(SDL_ttf REQUIRED)
# TODO(sirver): Do not globally include for all targets.
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
    world
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
