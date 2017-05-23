project (widelands)

cmake_minimum_required (VERSION 2.8.7)

include("${CMAKE_SOURCE_DIR}/cmake/WlFunctions.cmake")

option(OPTION_USE_GLBINDING "Use glbinding instead of GLEW" OFF)
option(OPTION_GLEW_STATIC "Use static GLEW Library" OFF)
option(OPTION_BUILD_WEBSITE_TOOLS "Build website-related tools" ON)
option(OPTION_BUILD_TRANSLATIONS "Build translations" ON)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Build directory and source directory must not be the same.")
endif (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

# Define the directory structure for installation - will be hardcoded in WL bin
# (through config.h). If not specified, we are going to use the directory where
# the executable is in. Also on Linux.
# Packagers (or people using make install) have to set this variable to an absolute path.
wl_set_if_unset(WL_INSTALL_BASEDIR ".")

# Define the directory structure for installation of the data files - will be hardcoded
# in WL bin (through config.h). If not specified, we are going to use the "data" directory
# below the directory where the executable is in. Also on Linux.
# Packagers (or people using make install) have to set this variable to an absolute path.
wl_set_if_unset(WL_INSTALL_DATADIR "./data")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
    message(FATAL_ERROR "Widelands needs GCC >= 4.8 to compile.")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.4)
    message(FATAL_ERROR "Clang version must be at least 3.4!")
  endif()
else()
  message(WARNING "You are using an unsupported compiler! Supported are Clang and GCC.")
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

if (WIN32)
  set (Boost_USE_STATIC_LIBS  ON)
  link_libraries(wsock32 ws2_32)
else()
  set (Boost_USE_STATIC_LIBS  OFF)
endif()
set (Boost_USE_MULTITHREADED ON)
set (Boost_DETAILED_FAILURE_MSG ON)
find_package(Boost 1.48
  COMPONENTS
    unit_test_framework
    regex
  REQUIRED
    system)

find_package (PythonInterp REQUIRED)

find_package(Gettext REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(ZLIB REQUIRED)
find_package(ICU REQUIRED)
if(OPTION_USE_GLBINDING)
  find_package(glbinding REQUIRED)
else()
  find_package(GLEW REQUIRED)
endif()

if (APPLE OR WIN32 OR
    CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR
    CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
  if (NOT CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")
    find_package(intl REQUIRED)
  endif()
endif()

# TODO(sirver): One day, this should be enabled. Then we have no more cycles in our dependencies....
# set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_NO_CYCLES ON)

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
  if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 6.3))
    message(STATUS "GCC 6.3.0 breaks -03, setting -02")
    set(WL_OPTIMIZE_FLAGS "-O2")
  else()
    set(WL_OPTIMIZE_FLAGS "-O3")
  endif ()
  set(WL_DEBUG_FLAGS "-DNDEBUG -DNOPARACHUTE")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 6.3))
    message(STATUS "GCC 6.3.0 breaks -03, setting -02")
    set(WL_OPTIMIZE_FLAGS "-O2")
  else()
    set(WL_OPTIMIZE_FLAGS "-O3")
  endif ()
  set(WL_DEBUG_FLAGS "-DNDEBUG -DNOPARACHUTE")
else()
  message(FATAL_ERROR "Unknown CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

wl_add_flag(WL_GENERIC_CXX_FLAGS "-std=c++11")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Weverything")

  # Disabled warnings that are overly verbose right now or just do not make sense.
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-c++98-compat")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-c++98-compat-pedantic")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-conversion")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-exit-time-destructors")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-global-constructors")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-padded")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-sign-conversion")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-missing-noreturn")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-implicit-fallthrough")

  # TODO(sirver): weak-vtables should be enabled, but leads to lot of errors right now.
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-weak-vtables")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-unreachable-code")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-documentation")

  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Werror=deprecated")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Werror=non-pod-varargs")

  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Qunused-arguments")
else()
  # Assuming GCC 4.8 or higher.
  if(WIN32)
    # This is needed for getenv().
    wl_add_flag(WL_GENERIC_CXX_FLAGS "-std=gnu++11")
  endif()

  # Because of: https://svn.boost.org/trac/boost/ticket/9240.
  wl_add_flag(WL_GENERIC_CXX_FLAGS "-fext-numeric-literals")

  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wall")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wpedantic")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wdeprecated-declarations")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wextra")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wformat")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wformat-nonliteral")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wformat-security")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wformat-y2k")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Winit-self")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Winvalid-pch")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wlogical-op")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wmissing-include-dirs")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-undef")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wold-style-cast")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Woverlength-strings")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wpacked")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wpointer-arith")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wredundant-decls")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wsign-promo")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wsync-nand")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wtrampolines")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wundef")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wunused")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wunused-macros")
endif()


# Turn some warnings into errors.
wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Werror=format-security")
wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Werror=return-type")
wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Werror=shadow")
wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Werror=uninitialized")

IF (WIN32)
  add_definitions(-DMINGW_HAS_SECURE_API)
  if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set (CMAKE_EXE_LINKER_FLAGS "-Wl,--large-address-aware" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)
    message (STATUS "Enabled large address awareness on mingw32")
  else (CMAKE_SIZEOF_VOID_P EQUAL 4)
    message (STATUS "Detected mingw32-w64")
  endif (CMAKE_SIZEOF_VOID_P EQUAL 4)
endif (WIN32)

# on BSD this must be explicitly linked
if (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
  # Not needed on Debian GNU/kFreeBSD..
  if (NOT CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")
    find_library(EXECINFO_LIBRARY NAMES execinfo)
  endif (NOT CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")

  # OpenBSD needs the X11 include directory in order to find GL/glu.h
  if (CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
    find_package(X11 REQUIRED)
    include_directories(${X11_INCLUDE_DIR})
  endif ()
endif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")

if (NOT DEFINED WL_VERSION)
  add_custom_target (
    BzrRevision ALL
    COMMAND ${CMAKE_COMMAND} -DWL_INSTALL_BASEDIR=${WL_INSTALL_BASEDIR} -DWL_INSTALL_DATADIR=${WL_INSTALL_DATADIR} -DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/BzrRevision.cmake
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
    COMMAND ${CMAKE_COMMAND -DWL_INSTALL_BASEDIR=${WL_INSTALL_BASEDIR} -DWL_VERSION=${WL_VERSION} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/InputRevision.cmake
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
  DESTINATION ${WL_INSTALL_BASEDIR}
  CONFIGURATIONS Debug;Release
  COMPONENT CoreVersionFile
)

# Installation.
add_subdirectory(doc)
if(OPTION_BUILD_TRANSLATIONS)
  add_subdirectory(po)
endif(OPTION_BUILD_TRANSLATIONS)
add_subdirectory(src)

# install files to the correct locations here
install(
  DIRECTORY
    data/campaigns
    data/i18n
    data/images
    data/scripting
    data/shaders
    data/tribes
    data/txts
    data/world
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT CoreDataFiles
)

install(
  DIRECTORY
    data/maps
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT MapFiles
)

install(
  DIRECTORY
    data/music
    data/sound
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release
  COMPONENT MusicFiles
)

install(
  FILES
    COPYING
    CREDITS
    ChangeLog
  DESTINATION ${WL_INSTALL_BASEDIR}
  CONFIGURATIONS Debug;Release
  COMPONENT CoreLicenseFiles
)

install(
  DIRECTORY
    doc
  DESTINATION ${WL_INSTALL_BASEDIR}
  CONFIGURATIONS Debug
  COMPONENT DocFiles
  PATTERN "CMakeLists.txt" EXCLUDE
)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${WL_INSTALL_DATADIR}/locale)

install(
  DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/locale/
  DESTINATION ${WL_INSTALL_DATADIR}/locale
  CONFIGURATIONS Debug;Release
  COMPONENT CoreLanguageFiles
)
