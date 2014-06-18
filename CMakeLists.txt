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

set (Boost_USE_STATIC_LIBS  OFF)
set (Boost_USE_MULTITHREADED ON)
set (Boost_DETAILED_FAILURE_MSG ON)
find_package(Boost 1.48
  COMPONENTS
    unit_test_framework
    regex
  REQUIRED)

find_package (PythonInterp REQUIRED)

# Check for opengl
# TODO Check for SDL_opengl.h and add to include path
find_package(OpenGL REQUIRED)

# OpenGL Headers are not needed directly. Instead SDL_opengl.h should be searched
find_package(GLEW REQUIRED)
add_definitions(${GLEW_EXTRA_DEFINITIONS})


find_package(ZLIB REQUIRED)
# TODO(sirver): Do not globally include for all targets.
include_directories(${ZLIB_INCLUDE_DIR})

find_package(SDL REQUIRED)
# TODO(sirver): Do not globally include for all targets.
include_directories(${SDL_INCLUDE_DIR})

find_package(Gettext REQUIRED)
find_package(PNG REQUIRED)
find_package(SDL_gfx REQUIRED)
find_package(SDL_image REQUIRED)
find_package(SDL_mixer REQUIRED)
find_package(SDL_net REQUIRED)
find_package(SDL_ttf REQUIRED)
find_package(ZLIB REQUIRED)

if (APPLE OR WIN32 OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  if (NOT CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")
    find_package(intl REQUIRED)
    # TODO(sirver): Do not globally include for all targets.
    include_directories(${INTL_INCLUDE_DIR})
  endif ()
endif()

set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_NO_CYCLES ON)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  find_path(FILE_WL_RELEASE "WL_RELEASE" ${CMAKE_CURRENT_SOURCE_DIR})
  if(${FILE_WL_RELEASE} STREQUAL "FILE_WL_RELEASE-NOTFOUND")
    set(CMAKE_BUILD_TYPE Debug)
  else()
    set(CMAKE_BUILD_TYPE Release)
  endif()
endif (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")

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


function(add_flag VAR_NAME FLAG)
  if (${VAR_NAME})
    set(${VAR_NAME} "${${VAR_NAME}} ${FLAG}" PARENT_SCOPE)
  else()
    set(${VAR_NAME} "${FLAG}" PARENT_SCOPE)
  endif()
endfunction()

add_flag(WL_GENERIC_CXX_FLAGS "-std=c++11")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Weverything")

  # Disabled warnings that are overly verbose right now or just do not make sense.
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-padded")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-c++98-compat")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-c++98-compat-pedantic")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-sign-conversion")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-conversion")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-exit-time-destructors")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-global-constructors")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Qunused-arguments")

  # TODO(sirver): weak-vtables should be enabled, but leads to lot of errors right now.
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-weak-vtables")
else()
  # Assuming GCC 4.7 or higher.
  if(WIN32)
    # TODO(sirver): theoretically, this should not be needed.
    add_flag(WL_GENERIC_CXX_FLAGS "-std=gnu++11")
  endif()

  # Because of: https://svn.boost.org/trac/boost/ticket/9240.
  add_flag(WL_GENERIC_CXX_FLAGS "-fext-numeric-literals")

  add_flag(WL_COMPILE_DIAGNOSTICS "-Wall")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wpedantic")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wdeprecated-declarations")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wextra")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wformat")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wformat-nonliteral")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wformat-security")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wformat-y2k")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Winit-self")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Winvalid-pch")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wlogical-op")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wmissing-include-dirs")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-undef")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wold-style-cast")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Woverlength-strings")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wpacked")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wpointer-arith")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wredundant-decls")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wshadow")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wsign-promo")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wswitch-default")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wsync-nand")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wtrampolines")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wundef")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wunused")
  add_flag(WL_COMPILE_DIAGNOSTICS "-Wunused-macros")
  add_flag(WL_COMPILE_DIAGNOSTICS "-fdiagnostics-show-option")
endif()

# Turn some warnings into errors.
add_flag(WL_COMPILE_DIAGNOSTICS "-Werror=uninitialized")

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

# Enable testing.
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

install (
  FILES ${CMAKE_CURRENT_BINARY_DIR}/VERSION
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT CoreVersionFile
)

# Installation.
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
