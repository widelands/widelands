# Install script for directory: /home/benedikt/Programmierung/widelands/amazons/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xExecutableFilesx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./widelands" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./widelands")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./widelands"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE EXECUTABLE FILES "/home/benedikt/Programmierung/widelands/amazons/build/src/widelands")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./widelands" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./widelands")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./widelands")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/ai/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/base/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/chat/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/economy/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/editor/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/game_io/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/graphic/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/io/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/logic/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/map_io/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/network/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/notifications/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/random/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/scripting/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/sound/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/third_party/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/ui_basic/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/ui_fsmenu/cmake_install.cmake")
  include("/home/benedikt/Programmierung/widelands/amazons/build/src/wui/cmake_install.cmake")

endif()

