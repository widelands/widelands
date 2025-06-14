cmake_minimum_required (VERSION 3.12)
cmake_policy(VERSION 3.12)

project (widelands)

message(STATUS "Using CMake version ${CMAKE_VERSION}")

# Detailed help is in README.md
# Please update it if you change anything here
option(OPTION_USE_GLBINDING "Use glbinding instead of GLEW" OFF)
option(OPTION_GLEW_STATIC "Use static GLEW Library" OFF)
option(OPTION_BUILD_WEBSITE_TOOLS "Build website-related tools" ON)
option(OPTION_BUILD_TESTS "Build tests" ON)
option(OPTION_BUILD_CODECHECK "Build codecheck" ON)
option(OPTION_BUILD_WINSTATIC "Build a static linked .exe on windows" OFF)
option(OPTION_TSAN "Build with ThreadSanitizer" OFF)
option(OPTION_FORCE_EMBEDDED_MINIZIP "Use embedded minizip sources" OFF)
option(USE_XDG "Follow XDG-Basedir specification" ON) # Enabled by default

set(CMAKE_FIND_FRAMEWORK LAST)

# Absolute path reference for submodules
set(WL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Add custom includes to override system libs
FILE(GLOB custom_include_directories auto_dependencies/*)
foreach(custom_include_dir ${custom_include_directories})
  if(IS_DIRECTORY "${custom_include_dir}")
    message(STATUS "Using custom include directory '${custom_include_dir}'.")
    list(INSERT CMAKE_INCLUDE_PATH 0 "${custom_include_dir}")
    include_directories(BEFORE SYSTEM "${custom_include_dir}")
  else()
    message(WARNING "Not a directory: '${custom_include_dir}' - Ignored.")
  endif()
endforeach()

include("${CMAKE_SOURCE_DIR}/cmake/UseSystemInfo.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/WlFunctions.cmake")

# This policy is not known to versions prior 3.27 and would result in errors,
# if set on such systems. This can be removed when cmake_minimum_required is set
# to 3.27 or newer  by using:
# cmake_policy(VERSION 3.27)
if(POLICY CMP0148)
  cmake_policy(SET CMP0148 NEW)
endif(POLICY CMP0148)
if(POLICY CMP0153)
  cmake_policy(SET CMP0153 NEW)
endif(POLICY CMP0153)
if(POLICY CMP0177)  # Added in CMake 3.31
  cmake_policy(SET CMP0177 NEW)
endif(POLICY CMP0177)

IF(USE_XDG AND NOT APPLE AND NOT WIN32)
  ADD_DEFINITIONS(-DUSE_XDG)
  message(STATUS "Building with XDG support")
ENDIF(USE_XDG AND NOT APPLE AND NOT WIN32)

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

# To override this, use '-DWL_INSTALL_BINDIR=<absolute path>' when configuring or change
# 'CMAKE_INSTALL_PREFIX'.
# Example: to have the bin installed to "/usr/games", just use '-DCMAKE_INSTALL_PREFIX=/usr'
# and don't specify '-DWL_INSTALL_BINDIR' when configuring.
# Note: 'CMAKE_INSTALL_PREFIX' defaults to /usr/local on UNIX and c:/Program Files/${PROJECT_NAME} on Windows
wl_set_if_unset(WL_INSTALL_BINDIR "${CMAKE_INSTALL_PREFIX}/games")

if(NOT CMAKE_C_COMPILER_LAUNCHER)
  find_program(CCACHE_PROGRAM ccache)
  if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM} CACHE STRING "C Compiler Launcher" FORCE)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM} CACHE STRING "CXX Compiler Launcher" FORCE)
  endif()
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
    message(FATAL_ERROR "Widelands needs GCC >= 8 to compile.")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
    message(FATAL_ERROR "Clang version must be at least 7!")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.4)
    message(FATAL_ERROR "MSVC version must be at least 19!")
  endif()
else()
  message(WARNING "You are using an unsupported compiler! Supported are Clang and GCC.")
endif()
message(STATUS "Picking up default CXX flags:
    - CMAKE_CXX_FLAGS='${CMAKE_CXX_FLAGS}'
    - CMAKE_CXX_FLAGS_RELEASE='${CMAKE_CXX_FLAGS_RELEASE}'
    - CMAKE_CXX_FLAGS_DEBUG='${CMAKE_CXX_FLAGS_DEBUG}'")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

if (WIN32)
  link_libraries(wsock32 ws2_32 bcrypt)
endif()
if (OPTION_BUILD_WINSTATIC)
  add_definitions(-DGLBINDING_STATIC_DEFINE -DNGHTTP2_STATICLIB)
  set (OPTION_GLEW_STATIC ON)
  if(MSVC)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
  else()
    set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  endif()
endif()

find_package (Python3 REQUIRED)

find_package(OpenGL REQUIRED)
find_package(PNG 1.6 REQUIRED)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(ZLIB REQUIRED)
if(${CMAKE_VERSION} VERSION_LESS 3.9.0)
    find_package(ICU_old REQUIRED)
else()
    find_package(ICU REQUIRED COMPONENTS uc dt)
endif()
if(OPTION_USE_GLBINDING)
  find_package(glbinding REQUIRED)
else()
  find_package(GLEW REQUIRED)
endif()

# Check that Asio is installed. Asio does not have (or need) a Find… CMake Module, so we "only" need to check if the header exists.
if(NOT asio_location OR asio_location STREQUAL "")
  unset(asio_location CACHE)
  find_path(asio_location "asio.hpp")
  if(asio_location)
    message(STATUS "Found Asio in '${asio_location}'.")
  else()
    message(FATAL_ERROR "Could NOT find Asio (missing: asio.hpp). Please run:\n./install-dependencies.sh\nwhich will attempt to install all missing dependencies automatically, or provide further instructions if this not supported on your system.")
  endif()
endif()
add_definitions(-DASIO_STANDALONE)

if (APPLE OR WIN32 OR
    CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR
    CMAKE_SYSTEM_NAME MATCHES "NetBSD" OR
    CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
endif()

# Disable no symbols warning on macOS, but only on versions where this is
# supported. It is not supported on OS X 10.7 (DARWIN_MAJOR_VERSION == 11).
if (APPLE)
    if (${DARWIN_MAJOR_VERSION} GREATER 11)
        SET(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
        SET(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
        SET(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
        SET(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
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
  set(WL_DEBUG_FLAGS "-DDEBUG")
  if(NOT MSVC)
    wl_add_flag(WL_DEBUG_FLAGS "-g")
    if(NOT OPTION_TSAN)
      option(OPTION_ASAN "Build with AddressSanitizer" ON)
    endif()
  endif()
elseif((CMAKE_BUILD_TYPE STREQUAL "Release") OR (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
  if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.5))
    message(STATUS "GCC >7.5.0 breaks -O3, setting -O2")
    set(WL_OPTIMIZE_FLAGS "-O2")
  elseif(MSVC)
    set(WL_OPTIMIZE_FLAGS "/O2")
  else()
    set(WL_OPTIMIZE_FLAGS "-O3")
  endif()
  if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.6))
    message(WARNING "GCC <7.6.0 is known to cause segfaults in release builds. Please consider upgrading to GCC version 7.6 or newer or using Clang. Adding selected overrides to suppress reporting of known bugs.")
    wl_add_flag(WL_OPTIMIZE_FLAGS "--param asan-stack=0")
  endif()
  set(WL_DEBUG_FLAGS "-DNDEBUG -DNOPARACHUTE")
  option(OPTION_ASAN "Build with AddressSanitizer" OFF)
elseif(CMAKE_BUILD_TYPE STREQUAL "None")
  message(STATUS "Not setting any default flags.")
else()
  message(FATAL_ERROR "Unknown CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

if(OPTION_TSAN)
  message(STATUS "Using ThreadSanitizer https://clang.llvm.org/docs/ThreadSanitizer.html")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-fsanitize=thread")
  set (CMAKE_EXE_LINKER_FLAGS "-fsanitize=thread" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)
  option(OPTION_ASAN "Build with AddressSanitizer" OFF)
else()
  message(STATUS "Not using ThreadSanitizer.")
endif(OPTION_TSAN)

if(OPTION_ASAN)
  message(STATUS "Using AddressSanitizer https://clang.llvm.org/docs/AddressSanitizer.html")
  # See https://clang.llvm.org/docs/AddressSanitizer.html
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-fsanitize=address")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "-fno-omit-frame-pointer")
  set (CMAKE_EXE_LINKER_FLAGS "-fsanitize=address" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)
  add_definitions(-DUSE_ASAN)
else()
  message(STATUS "Not using AddressSanitizer.")
endif(OPTION_ASAN)

if(OPTION_ASAN AND OPTION_TSAN)
  message(FATAL_ERROR "OPTION_ASAN and OPTION_TSAN cannot be used together")
endif()

# This is set to avoid linker errors when using GLVND-libs on Linux
if("${OpenGL_GL_PREFERENCE}" STREQUAL "GLVND")
   link_libraries(OpenGL::GL)
   add_compile_definitions(WL_USE_GLVND)
   message(STATUS "Adding linker flags for GLVND.")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(warn_disable_flags
    "-Weverything"

    # Disabled warnings that are overly verbose right now or just do not make sense.
    "-Wno-c++98-compat"
    "-Wno-c++98-compat-pedantic"
    "-Wno-conversion"
    "-Wno-exit-time-destructors"
    "-Wno-global-constructors"
    "-Wno-padded"
    "-Wno-sign-conversion"
    "-Wno-missing-noreturn"

    # It is impossible to write code that both GCC and Clang will like,
    # so we have to switch off the warning for one of them.
    # http://clang-developers.42468.n3.nabble.com/Question-on-Wswitch-enum-td4025927.html
    "-Wno-switch-enum"
    "-Wno-covered-switch-default"

    # TODO(sirver: weak-vtables should be enabled, but leads to lot of errors right now.
    "-Wno-weak-vtables"
    "-Wno-unreachable-code"
    "-Wno-documentation"
    "-Qunused-arguments"
    "-Wint-to-void-pointer-cast"
  )
  foreach(flag IN LISTS warn_disable_flags)
    wl_add_flag(WL_COMPILE_DIAGNOSTICS ${flag})
  endforeach()

  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.8)
    wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Winconsistent-missing-override")
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 16)
    wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-unsafe-buffer-usage")
  endif()
elseif(MSVC)
  wl_add_flag(WL_GENERIC_CXX_FLAGS "/std:c++17")
  wl_add_flag(WL_GENERIC_C_LIBRARY_FLAGS "/std:c11")
  # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/
  # Disabled warnings that are overly verbose right now or just do not make sense.
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "/wd4244")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "/wd4267")
  wl_add_flag(WL_COMPILE_DIAGNOSTICS "/wd4018")
  # Use correct code page
  wl_add_flag(WL_GENERIC_CXX_FLAGS "/utf-8")
else()
  # Assuming GCC 4.8 or higher.
  if(WIN32)
    # This is needed for getenv().
    wl_add_flag(WL_GENERIC_CXX_FLAGS "-std=gnu++17")
    wl_add_flag(WL_GENERIC_C_LIBRARY_FLAGS "-std=c11")
    if (OPTION_BUILD_WINSTATIC)
      set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ ${CMAKE_CXX_STANDARD_LIBRARIES}")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic")
    endif()
  else()
    # SDL and MinGW both declare 'unsigned int __builtin_ia32_crc32qi(unsigned int, unsigned char)', resulting in lots of warnings. So, we can't have this flag in Windows.
    wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wredundant-decls")
  endif()

  set(warn_flags
    "-Wall"
    "-Wpedantic"
    "-Wdeprecated-declarations"
    "-Wextra"
    "-Wformat"
    "-Wformat-nonliteral"
    "-Wformat-security"
    "-Wformat-y2k"
    "-Winit-self"
    "-Winvalid-pch"
    "-Wmissing-include-dirs"
    "-Wno-undef"
    "-Wold-style-cast"
    "-Woverlength-strings"
    "-Wpacked"
    "-Wpointer-arith"
    "-Wsign-promo"
    "-Wundef"
    "-Wunused"
    "-Wunused-macros"
    "-fno-elide-constructors" # Detect invalid constexpr initialization
  )
  foreach(flag IN LISTS warn_flags)
    wl_add_flag(WL_COMPILE_DIAGNOSTICS ${flag})
  endforeach()
  if (NOT APPLE)
    wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wlogical-op")
    wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wsync-nand")
    wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wtrampolines")
  endif()
endif()

# Some compilers need an extra library for filesystem support, while others don't know about it.
# If the detection is incorrect, you can override it by setting `-DNEEDS_EXTERNAL_FILESYSTEM=ON|OFF` explicitly.
if (NOT DEFINED NEEDS_EXTERNAL_FILESYSTEM)
  if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1))
    set(NEEDS_EXTERNAL_FILESYSTEM ON CACHE BOOL "Use linker flag for stdc++fs")
  else()
    set(NEEDS_EXTERNAL_FILESYSTEM OFF CACHE BOOL "Don't use linker flag for stdc++fs")
  endif()
endif()
if (NEEDS_EXTERNAL_FILESYSTEM)
  message(STATUS "Linking with external stdc++fs")
endif()

if(NOT MSVC)
  if((CMAKE_BUILD_TYPE STREQUAL "Release") OR (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
    message(STATUS "Compiler warnings will be ignored.")
  elseif(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") AND (
         ((NOT (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.0)) AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.2))
         OR (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.0)) AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.1)))
    # Skip this on g++ versions 12.0, 12.1, and 13.0 because of false-positives from system headers.
    message(WARNING "This compiler is known to cause false-positive warnings.")
  else()
    # Turn some warnings into errors.
    message(STATUS "Warnings will be treated as errors.")
    wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Werror")
    if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0))
      # Silence some false positives on older g++
      message(STATUS "Disabling known false-positive warnings for this compiler.")
      wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-error=unused-variable")
      wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-error=unused-but-set-parameter")
    endif()
    if (APPLE)  # Our Mac CI needs these
      message(STATUS "Disabling known false-positive warnings for this environment.")
      wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-error=poison-system-directories")
      wl_add_flag(WL_COMPILE_DIAGNOSTICS "-Wno-error=disabled-macro-expansion")
    endif()
  endif()

  wl_add_flag(WL_GENERIC_CXX_FLAGS "-std=c++17")
  wl_add_flag(WL_GENERIC_C_LIBRARY_FLAGS "-std=c11")
endif()

# Cross-compile-unit optimization slows linking significantly.
# Therefore, impatient user may want to prevent that.
if(USE_FLTO_IF_AVAILABLE STREQUAL "yes")
# Cross-compile-unit optimization not supported by gcc with debug on!
# Therefore, this may only be done with Release builds.
if((CMAKE_BUILD_TYPE STREQUAL "Release") OR (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
  # Only tested on Linux. Might work also on other platforms.
  # If tested and found good, remove this check
  if (CMAKE_SYSTEM MATCHES "Linux")
    # Usage of -flto tested only with gcc 8.3 and 9.3 (2020-04); when in doubt, leaving it out..
    if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 8.0))
      #
      # The execution jumps between compile units rather often. -flot allows optimization across
      # comple units. Binary size reduced by >15% on tested environments. This does not directly
      # translate to execution speed, but might still be noticeable.
      #
      # This is not related to -O2 or -O3 discussion, and not enable by either.
      #
      # If using lto, it needs to happen both during compile and link, else no benefits.
      wl_add_flag(WL_GENERIC_CXX_FLAGS "-flto")
      SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -flto")
      message(STATUS "Enabling link-time optimizations")
    endif()
  endif(CMAKE_SYSTEM MATCHES "Linux")
endif((CMAKE_BUILD_TYPE STREQUAL "Release") OR (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
endif(USE_FLTO_IF_AVAILABLE STREQUAL  "yes")

IF (WIN32)
  add_definitions(-D_WIN32_WINNT=0x0601)
  if(MSVC)
    add_definitions(-DWIN32_LEAN_AND_MEAN -D__STDC_FORMAT_MACROS -DNOMINMAX)
    # Needed for minizip
    add_definitions(-Dor=|| -Dand=&& -Dnot=!)
  else()
    add_definitions(-DMINGW_HAS_SECURE_API -DWIN32_LEAN_AND_MEAN -D__STDC_FORMAT_MACROS -D__USE_MINGW_ANSI_STDIO)
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
      set (CMAKE_EXE_LINKER_FLAGS "-Wl,--large-address-aware" CACHE STRING "Set by widelands CMakeLists.txt" FORCE)
      message (STATUS "Enabled large address awareness on mingw32")
      message (WARNING "mingw compiler for 32 bit is deprecated, consider switching to 64-bit!")
      set(MINGW_SDL_IMG_DEPS webp webpdemux webpdecoder jbig deflate Lerc jxl hwy avif aom yuv dav1d sharpyuv ntdll)
    else (CMAKE_SIZEOF_VOID_P EQUAL 4)
      message (STATUS "Detected mingw32-w64")
      set(MINGW_SDL_IMG_DEPS webp webpdemux webpdecoder jbig deflate Lerc jxl hwy avif aom rav1e yuv dav1d SvtAv1Enc sharpyuv ntdll)
    endif (CMAKE_SIZEOF_VOID_P EQUAL 4)
  endif()
else()
  add_definitions(-DLUA_USE_POSIX)
endif()

if(OPTION_BUILD_WINSTATIC)
  # Add required libraries for statical linking
  wl_add_static_libs(SDL_IMG_STATIC_LIBS tiff jpeg lzma)
  wl_add_static_libs(SDL_MIXER_STATIC_LIBS opusfile opus FLAC vorbisfile vorbis ogg mpg123 shlwapi)
  wl_add_static_libs(SDL_TTF_STATIC_LIBS freetype bz2 graphite2 usp10 dwrite harfbuzz freetype rpcrt4)
  wl_add_static_libs(SDL_STATIC_LIBS dinput8 shell32 setupapi advapi32 uuid version oleaut32 ole32 imm32 winmm gdi32 user32)
  set(BROTLI brotlidec brotlicommon brotlienc)
  if(MSVC)
    wl_add_static_libs(SDL_IMG_STATIC_LIBS libwebp)
    wl_add_static_libs(SDL_STATIC_LIBS charset ${BROTLI})
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      string(REPLACE "/MDd" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
      wl_add_flag(TARGET_COMPILE_FLAGS "/MTd /EHsc")
    else()
      string(REPLACE "/MD" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
      wl_add_flag(TARGET_COMPILE_FLAGS "/MT /EHsc")
    endif()
  else()
    wl_add_static_libs(SDL_IMG_STATIC_LIBS ${MINGW_SDL_IMG_DEPS})
    wl_add_static_libs(SDL_MIXER_STATIC_LIBS ssp xmp wavpack)
    wl_add_static_libs(SDL_STATIC_LIBS mingw32 dxerr8 zstd ${BROTLI})
  endif(MSVC)
endif(OPTION_BUILD_WINSTATIC)

if(NOT OPTION_ASAN)
  # Allow self-diagnosis for crashes if the system supports it (uses a GNU extension)
  find_path(has_execinfo "execinfo.h")
  if(has_execinfo)
    message(STATUS "Adding crash self-diagnosis handler.")
    add_definitions(-DPRINT_SEGFAULT_BACKTRACE)
    if(NOT WIN32)
      # Needed for human-readable-ish symbolizing; doesn't hurt performance, see https://stackoverflow.com/q/12634114
      SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
    endif()
  endif()
endif()

# on BSD this must be explicitly linked
if (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR
    CMAKE_SYSTEM_NAME MATCHES "NetBSD" OR
    CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
  # Not needed on Debian GNU/kFreeBSD..
  if (NOT CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")
    find_library(EXECINFO_LIBRARY NAMES execinfo)
  endif (NOT CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")

  # OpenBSD needs the X11 include directory in order to find GL/glu.h
  if (CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
    find_package(X11 REQUIRED)
    include_directories(${X11_INCLUDE_DIR})
  endif ()
endif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR
       CMAKE_SYSTEM_NAME MATCHES "NetBSD" OR
       CMAKE_SYSTEM_NAME MATCHES "OpenBSD")

if (NOT DEFINED WL_VERSION)
  include (${CMAKE_SOURCE_DIR}/cmake/BzrRevision.cmake)
else (NOT DEFINED WL_VERSION)
  add_custom_target (
    InputRevision ALL
    COMMAND ${CMAKE_COMMAND} -DWL_INSTALL_BASEDIR=${WL_INSTALL_BASEDIR} -DWL_VERSION=${WL_VERSION} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/InputRevision.cmake
  )
endif (NOT DEFINED WL_VERSION)

# Enable testing.
if (OPTION_BUILD_TESTS)
  include(CTest)
  enable_testing()

  # Run a test after a normal compile. This magic is needed as 'make test' will
  # not rebuild tests:
  # https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
  add_custom_target(_run_all_tests ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS wl_tests
  )
endif (OPTION_BUILD_TESTS)

install (
  FILES ${CMAKE_CURRENT_BINARY_DIR}/VERSION
  DESTINATION ${WL_INSTALL_BASEDIR}
  CONFIGURATIONS Debug;Release;RelWithDebInfo;None
  COMPONENT CoreVersionFile
)

# TODO(Nordfriese): Use this consistently in places in code that use various other tests for endianness
include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
  message(STATUS "Byte order is big-endian")
  ADD_DEFINITIONS(-DWORDS_BIGENDIAN)
else()
  message(STATUS "Byte order is little-endian")
endif()

# Installation.
add_subdirectory(doc)
add_subdirectory(src)

# install files to the correct locations here
install(
  FILES
    data/datadirversion
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release;RelWithDebInfo;None
  COMPONENT VersionFile
)

install(
  DIRECTORY
    data/ai
    data/campaigns
    data/i18n
    data/images
    data/scripting
    data/shaders
    data/templates
    data/tribes
    data/txts
    data/world
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release;RelWithDebInfo;None
  COMPONENT CoreDataFiles
)

install(
  DIRECTORY
    data/maps
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release;RelWithDebInfo;None
  COMPONENT MapFiles
)

install(
  DIRECTORY
    data/music
    data/sound
  DESTINATION ${WL_INSTALL_DATADIR}
  CONFIGURATIONS Debug;Release;RelWithDebInfo;None
  COMPONENT MusicFiles
)

install(
  FILES
    COPYING
    CREDITS
    ChangeLog
  DESTINATION ${WL_INSTALL_BASEDIR}
  CONFIGURATIONS Debug;Release;RelWithDebInfo;None
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

if (UNIX AND NOT APPLE)
  add_subdirectory(xdg)
endif (UNIX AND NOT APPLE)

# uninstall target
# see https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#can-i-do-make-uninstall-with-cmake
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake)
endif()

