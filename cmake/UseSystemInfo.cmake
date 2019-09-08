# - Print CMake and OS distribution version
# 
# This file comes courtesy of opm-project.org
# https://github.com/OPM/opm-common/blob/master/cmake/Modules/UseSystemInfo.cmake
function (system_info)
  message (STATUS "CMake version: ${CMAKE_VERSION}")
  if (CMAKE_SYSTEM MATCHES "Linux")
    distro_name (DISTRO_NAME)
    message (STATUS "Linux distribution: ${DISTRO_NAME}")
  elseif (CMAKE_SYSTEM MATCHES "Darwin")
    sw_vers (OS_VERSION)
    message (STATUS "${OS_VERSION}")
  else (CMAKE_SYSTEM MATCHES "Linux")
    message (STATUS "Operating system: ${CMAKE_SYSTEM}")
  endif (CMAKE_SYSTEM MATCHES "Linux")

  target_architecture (TARGET_CPU)
  message (STATUS "Target architecture: ${TARGET_CPU}")
endfunction (system_info)

function (sw_vers varname)
  # query operating system for information
  exec_program (sw_vers OUTPUT_VARIABLE _vers)
  # split multi-line into various fields
  string (REPLACE "\n" ";" _vers "${_vers}")
  string (REPLACE ":" ";" _vers "${_vers}")
  # get the various fields
  list (GET _vers 1 _prod_name)
  list (GET _vers 3 _prod_vers)
  list (GET _vers 5 _prod_build)
  # remove extraneous whitespace
  string (STRIP "${_prod_name}" _prod_name)
  string (STRIP "${_prod_vers}" _prod_vers)
  string (STRIP "${_prod_build}" _prod_build)
  # assemble result variable
  set (${varname} "${_prod_name} version: ${_prod_vers} (${_prod_build})" PARENT_SCOPE)
endfunction (sw_vers varname)

# probe various system files that may be found
function (distro_name varname)
  file (GLOB has_os_release /etc/os-release)
  file (GLOB has_lsb_release /etc/lsb-release)
  file (GLOB has_sys_release /etc/system-release)
  file (GLOB has_redhat_release /etc/redhat-release)
  set (_descr)
  # start with /etc/os-release,
  # see <http://0pointer.de/blog/projects/os-release.html>
  if (NOT has_os_release STREQUAL "")
    read_release (PRETTY_NAME FROM /etc/os-release INTO _descr)
  # previous "standard", used on older Ubuntu and Debian
  elseif (NOT has_lsb_release STREQUAL "")
    read_release (DISTRIB_DESCRIPTION FROM /etc/lsb-release INTO _descr)
  endif (NOT has_os_release STREQUAL "")
  # RHEL/CentOS etc. has just a text-file
  if (NOT _descr)
    if (NOT has_sys_release STREQUAL "")
      file (READ /etc/system-release _descr)
    elseif (NOT has_redhat_release STREQUAL "")
      file (READ /etc/redhat-release _descr)
    else (NOT has_sys_release STREQUAL "")
      # no yet known release file found
      set (_descr "unknown")
    endif (NOT has_sys_release STREQUAL "")
  endif (NOT _descr)
  # return from function (into appropriate variable)
  string (STRIP "${_descr}" _descr)
  set (${varname} "${_descr}" PARENT_SCOPE)
endfunction (distro_name varname)

# read property from the newer /etc/os-release
function (read_release valuename FROM filename INTO varname)
  file (STRINGS ${filename} _distrib
    REGEX "^${valuename}="
    )
  string (REGEX REPLACE
    "^${valuename}=\"?\(.*\)" "\\1" ${varname} "${_distrib}"
    )
  # remove trailing quote that got globbed by the wildcard (greedy match)
  string (REGEX REPLACE
    "\"$" "" ${varname} "${${varname}}"
    )
  set (${varname} "${${varname}}" PARENT_SCOPE)
endfunction (read_release valuename FROM filename INTO varname)

# the following code is adapted from commit f7467762 of the code at
# <https://github.com/petroules/solar-cmake/blob/master/TargetArch.cmake>
# which is Copyright (c) 2012 Petroules Corporation, and which at the
# time of download (2013-04-07 12:30 CET) is made available with a BSD license.
#
# it attempts to compile a program which detects the architecture from the
# preprocessor symbols and communicate this back to us through an error message(!)
function (target_architecture output_var)
  # OS X is capable of building for *several* architectures at once in
  # the Mach-O binary, and there is a variable that tells us which those
  # are, but they may be in any order, so they must be normalized
  if (APPLE AND CMAKE_OSX_ARCHITECTURES)
    # detect each of the possible candidates as a separate flag
    set (osx_arch_list i386 x86_64)
    foreach (osx_arch IN ITEMS ${CMAKE_OSX_ARCHITECTURES})
      foreach (candidate IN LISTS osx_arch_list)
        if ("${osx_arch}" STREQUAL "${candidate}")
          set (osx_arch_${candidate} TRUE)
        endif ("${osx_arch}" STREQUAL "${candidate}")
      endforeach (candidate)
    endforeach (osx_arch)

    # add all architectures back in normalized order
    foreach (candidate IN LISTS osx_arch_list)
      if (osx_arch_${candidate})
        list (APPEND ARCH ${candidate})
      endif (osx_arch_${candidate})
    endforeach (candidate)

  else (APPLE AND CMAKE_OSX_ARCHITECTURES)
    # use the preprocessor defines to determine which target architectures
    # that are available
    set (arch_c_src "
      #if defined(__arm__) || defined(__TARGET_ARCH_ARM)
      #  if defined(__ARM_ARCH_7__) \\
         || defined(__ARM_ARCH_7A__) \\
         || defined(__ARM_ARCH_7R__) \\
         || defined(__ARM_ARCH_7M__) \\
         || (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM-0 >= 7)
      #    error cmake_ARCH armv7
      #  elif defined(__ARM_ARCH_6__) \\
           || defined(__ARM_ARCH_6J__) \\
           || defined(__ARM_ARCH_6T2__) \\
           || defined(__ARM_ARCH_6Z__) \\
           || defined(__ARM_ARCH_6K__) \\
           || defined(__ARM_ARCH_6ZK__) \\
           || defined(__ARM_ARCH_6M__) \\
           || (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM-0 >= 6)
      #    error cmake_ARCH armv6
      #  elif defined(__ARM_ARCH_5TEJ__) \\
          || (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM-0 >= 5)
      #    error cmake_ARCH armv5
      #  else
      #    error cmake_ARCH arm
      #  endif
      #elif defined(__i386) \\
         || defined(__i386__) \\
         || defined(_M_IX86)
      #  error cmake_ARCH i386
      #elif defined(__x86_64) \\
         || defined(__x86_64__) \\
         || defined(__amd64) \\
         || defined(_M_X64)
      #  error cmake_ARCH x86_64
      #elif defined(__ia64) \\
         || defined(__ia64__) \\
         || defined(_M_IA64)
      #  error cmake_ARCH ia64
      #elif defined(__ppc__) \\
         || defined(__ppc) \\
         || defined(__powerpc__) \\
         || defined(_ARCH_COM) \\
         || defined(_ARCH_PWR) \\
         || defined(_ARCH_PPC) \\
         || defined(_M_MPPC) \\
         || defined(_M_PPC)
      #  if defined(__ppc64__) \\
         || defined(__powerpc64__) \\
         || defined(__64BIT__)
      #    error cmake_ARCH ppc64
      #  else
      #    error cmake_ARCH ppc
      #  endif
      #else
      #  error cmake_ARCH unknown
      #endif
      ")
    
    # write a temporary program that can be compiled to get the result
    set (tmp_dir "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp")
    set (arch_c "${tmp_dir}/arch.c")
    file (WRITE "${arch_c}" "${arch_c_src}")
    try_compile (
      compile_result_unused
      "${tmp_dir}"
      "${arch_c}"
      CMAKE_FLAGS CMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      OUTPUT_VARIABLE ARCH
      )

    # parse the architecture name from the compiler output
    string (REGEX MATCH "cmake_ARCH ([a-zA-Z0-9_]+)" ARCH "${ARCH}")

    # get rid of the value marker leaving just the architecture name
    string (REPLACE "cmake_ARCH " "" ARCH "${ARCH}")

    # if we are compiling with an unknown architecture this variable should
    # already be set to "unknown" but in the case that it's empty (i.e. due
    # to a typo in the code), then set it to unknown
    if (NOT ARCH)
      set (ARCH "unknown")
    endif()
  endif()

  set (${output_var} "${ARCH}" PARENT_SCOPE)
endfunction()
