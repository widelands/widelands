project (widelands)

# possible configurations:
# set them via cmake -DVARIABLE=VALUE
# CMAKE_BUILD_TYPE:
#     Default is "Debug", Distributors should use "Release"
# WL_VERSION_MAJOR, WL_VERSION_MINOR (, WL_VERSION_PATCH):
#     Default is using SVN revision, Distributors should use correct version
#     Attention: We are currently only using MAJOR and MINOR.
#     If you try to set WL_VERSION directly, it will ignore your setting.

cmake_minimum_required (VERSION 2.6)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Build-directory and source-directory are the same!")
endif (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

if (NOT DEFINED CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Debug)
endif (NOT DEFINED CMAKE_BUILD_TYPE)

set (CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})


# CMAKE only defines "-g", but we need -DDEBUG also, and we need -DNOPARACHUTE (for SDL) in Debug
set (CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG -DNOPARACHUTE" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)

# CMAKE defines "-O3 -DNDEBUG", but we better say -O2 (see gcc manual)
set (CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)

# we don't want someone to set this directly...
unset (WL_VERSION CACHE)

# ...but using MAJOR, MINOR and PATCH
# (BUT: we only use MAJOR and MINOR for the moment; PATCH is set to 0)
if (DEFINED WL_VERSION_MAJOR)
  set (WL_VERSION ${WL_VERSION_MAJOR})
  set (CPACK_PACKAGE_VERSION_MAJOR ${WL_VERSION_MAJOR})
  if (DEFINED WL_VERSION_MINOR)
    set (WL_VERSION ${WL_VERSION}.${WL_VERSION_MINOR})
    set (CPACK_PACKAGE_VERSION_MINOR ${WL_VERSION_MINOR})
    if (DEFINED WL_VERSION_PATCH)
      set (WL_VERSION ${WL_VERSION}.${WL_VERSION_PATCH})
      set (CPACK_PACKAGE_VERSION_PATCH ${WL_VERSION_PATCH})
    else (DEFINED WL_VERSION_PATCH)
      set (CPACK_PACKAGE_VERSION_PATCH 0)
    endif (DEFINED WL_VERSION_PATCH)
  else (DEFINED WL_VERSION_MINOR)
    set (CPACK_PACKAGE_VERSION_MINOR 0)
  endif (DEFINED WL_VERSION_MINOR)
endif (DEFINED WL_VERSION_MAJOR)

if (NOT DEFINED WL_VERSION)
  find_package (PythonInterp REQUIRED)
  execute_process (
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/build/scons-tools/detect_revision.py
    OUTPUT_VARIABLE WL_VERSION
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif (NOT DEFINED WL_VERSION)
message ("Version of Widelands Build is ${WL_VERSION}(${CMAKE_BUILD_TYPE})")

set (WL_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set (WL_INSTALL_BINDIR ${WL_INSTALL_PREFIX}/bin/)
set (WL_INSTALL_DATADIR ${WL_INSTALL_PREFIX}/share/widelands/)
# is this needed?
set (WL_INSTALL_LOCALEDIR ${WL_INSTALL_DATADIR}/locale/)

set (WL_SOURCE_CHECKER ${CMAKE_CURRENT_SOURCE_DIR}/cmake/codecheck/CodeCheck.py)

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

IF (NOT WIN32)
# TODO
  if (CMAKE_BUILD_TYPE STREQUAL Debug)
    find_package(Doxygen REQUIRED)
    include(Documentation)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/sourcecode)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY IMMEDIATE)
    add_custom_target(doc COMMAND ${DOXYGEN_EXECUTABLE} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  endif(CMAKE_BUILD_TYPE STREQUAL Debug)
ENDIF (NOT WIN32)

IF (WIN32)
  SET(GUI_TYPE WIN32)
ENDIF (WIN32)

include_directories(src ${CMAKE_CURRENT_BINARY_DIR}/src)

#configure_file (src/config.h.cmake src/config.h)
#configure_file (src/build_info.cc.cmake src/build_info.cc)

#add_subdirectory(po)
add_subdirectory(pics)
add_subdirectory(src)

# install files to the correct locations here
install(
  DIRECTORY
    campaigns
    fonts
    global
    maps
    music
    pics
    sound
    tribes
    txts
    worlds
    po
#  locale
  DESTINATION share/widelands
  CONFIGURATIONS Debug;Release
  COMPONENT CoreDataFiles
  PATTERN "*.svn*" EXCLUDE
  PATTERN "SCons*" EXCLUDE
)

#if (CMAKE_BUILD_TYPE STREQUAL Debug)
  install(
    DIRECTORY
      doc
    DESTINATION share/widelands
    CONFIGURATIONS Debug
    COMPONENT DocFiles
    PATTERN "*.svn*" EXCLUDE
    PATTERN "SCons*" EXCLUDE
  )
#endif()

set (CPACK_PACKAGE_FILE_NAME widelands-${WL_VERSION})
set (CPACK_COMPONENTS_ALL ExecutableFiles CoreDataFiles DocFiles)
set (CPACK_GENERATOR TGZ)
set (CPACK_SOURCE_GENERATOR TGZ)
include(CPack)
