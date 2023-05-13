# Utilize cmake_parse_arguments standard function to parse for common arguments.
include(CMakeParseArguments)
macro(_parse_common_args ARGS)
  set(OPTIONS
    THIRD_PARTY  # Is a third party lib. Less warnings, no codecheck.
    C_LIBRARY # Pure C library. No CXX flags.
    WIN32 # Windows binary/library.
    USES_ATOMIC
    USES_ICU
    USES_INTL
    USES_MINIZIP
    USES_OPENGL
    USES_PNG
    USES_SDL2
    USES_SDL2_IMAGE
    USES_SDL2_MIXER
    USES_SDL2_TTF
    USES_ZLIB
  )
  set(ONE_VALUE_ARG )
  set(MULTI_VALUE_ARGS SRCS DEPENDS)
  cmake_parse_arguments(ARG "${OPTIONS}" "${ONE_VALUE_ARG}" "${MULTI_VALUE_ARGS}"
    ${ARGS}
  )
endmacro(_parse_common_args)

# Set variable VAR to VALUE if it is not set or empty. Does nothing if already set.
macro(wl_set_if_unset VAR VALUE)
  if (NOT ${VAR} OR ${VAR} STREQUAL "")
	  set (${VAR} ${VALUE})
  endif()
endmacro(wl_set_if_unset)

# Add DIR as include directores for TARGET. Depending on cmake version the way how
# it works is different.
function(wl_include_directories TARGET DIR)
  _include_directories_internal(${TARGET} ${DIR} FALSE)
endfunction(wl_include_directories TARGET DIR)

# Add DIR as SYSTEM include directores for TARGET. Depending on cmake version the way how
# it works is different. SYSTEM includes silence warnings for included headers etc.
function(wl_include_system_directories TARGET DIR)
  _include_directories_internal(${TARGET} ${DIR} TRUE)
endfunction(wl_include_system_directories TARGET DIR)

# Search the given libraries and write their paths into VAR_NAME.
function(wl_add_static_libs VAR_NAME)
    set(STATIC_LIBS ${${VAR_NAME}})
    foreach(LIB IN ITEMS ${ARGN})
        find_library(${LIB}_STATIC NAMES ${LIB} REQUIRED)
        list(APPEND STATIC_LIBS ${${LIB}_STATIC})
    endforeach()
    # Make it known to caller
    set(${VAR_NAME} ${STATIC_LIBS} PARENT_SCOPE)
endfunction()

# Add common compile tasks, like includes and libraries to link against for third party
# libraries, and codecheck hook for sources.
macro(_common_compile_tasks)
  if (NOT ARG_C_LIBRARY)
    set(TARGET_COMPILE_FLAGS "${TARGET_COMPILE_FLAGS} ${WL_GENERIC_CXX_FLAGS}")

    # This is needed for header only libraries. While they do not really mean
    # anything for cmake, they are useful to make dependencies explicit.
    set_target_properties(${NAME} PROPERTIES LINKER_LANGUAGE CXX)
  endif()

  set(TARGET_COMPILE_FLAGS "${TARGET_COMPILE_FLAGS} ${WL_OPTIMIZE_FLAGS} ${WL_DEBUG_FLAGS}")
  if (OPTION_BUILD_WINSTATIC AND NOT MSVC)
    set(TARGET_LINK_FLAGS "-static")
  endif()

  if(ARG_THIRD_PARTY)
    # Disable all warnings for third_party.
    set(TARGET_COMPILE_FLAGS "${TARGET_COMPILE_FLAGS} -w")
  else()
    foreach(SRC ${ARG_SRCS})
      wl_run_codecheck(${NAME} ${SRC})
    endforeach(SRC)

    set(TARGET_COMPILE_FLAGS "${TARGET_COMPILE_FLAGS} ${WL_COMPILE_DIAGNOSTICS}")
  endif()

  set_target_properties(${NAME} PROPERTIES COMPILE_FLAGS ${TARGET_COMPILE_FLAGS})

  if(NOT ARG_THIRD_PARTY)
    # src/ is the base for all of our includes. The binary one is for generated files.
    wl_include_directories(${NAME} ${CMAKE_SOURCE_DIR}/src)
    wl_include_directories(${NAME} ${CMAKE_BINARY_DIR}/src)
    if(MINIZIP_STATIC_LIBRARIES)
      wl_include_directories(${NAME} ${MINIZIP_INCLUDE_DIRS})
      target_compile_options(${NAME} PUBLIC ${MINIZIP_CFLAGS})
      if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag("-Wno-reserved-id-macro" HAVE_FLAG)
        if (HAVE_FLAG)
          target_compile_options(${NAME} PUBLIC "-Wno-reserved-id-macro")
        endif()
      endif()
    else()
      wl_include_directories(${NAME} ${CMAKE_SOURCE_DIR}/src/third_party/minizip)
    endif()
  endif()

  if(ARG_USES_MINIZIP)
      if(MINIZIP_STATIC_LIBRARIES)
          target_link_libraries(${NAME} minizip)
          message(STATUS "Link ${NAME} with minizip")
      else()
          target_link_libraries(${NAME} third_party_minizip)
          message(STATUS "Link ${NAME} with third_party_minizip")
      endif()
  endif()

  if(ARG_USES_ATOMIC AND NOT APPLE AND ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
    # clang on linux needs explicit linkage against standard library atomic
    target_link_libraries(${NAME} atomic)
  endif()

  if(ARG_USES_ZLIB)
    target_link_libraries(${NAME} ZLIB::ZLIB)
  endif()

  # OpenGL and GLEW are one thing for us. If you use the one, you also use the
  # other. We always add definitions, because add_definition() is not
  # transitive and therefore some dependent targets do not set them properly.
  # And a few -D do not hurt anything.
  if(OPTION_USE_GLBINDING)
    add_definitions("-DUSE_GLBINDING")
  elseif(${CMAKE_VERSION} VERSION_LESS 3.9.0)
    add_definitions(${GLEW_EXTRA_DEFINITIONS})
  endif()

  if(ARG_USES_OPENGL)
    if(OPTION_USE_GLBINDING)
      # Early versions of glbinding defined GLBINDING_INCLUDES, newer use
      # cmake's module system.
      if(DEFINED GLBINDING_INCLUDES)
        wl_include_system_directories(${NAME} ${GLBINDING_INCLUDES})
        target_link_libraries(${NAME} ${GLBINDING_LIBRARIES})
      else()
        target_link_libraries(${NAME} glbinding::glbinding)
      endif()
    else()
      if(${CMAKE_VERSION} VERSION_LESS 3.9.0)
        wl_include_system_directories(${NAME} ${GLEW_INCLUDE_DIR})
        target_link_libraries(${NAME} ${GLEW_LIBRARY})
        target_link_libraries(${NAME} ${OPENGL_gl_LIBRARY})
      else()
        if (OPTION_BUILD_WINSTATIC)
          target_link_libraries(${NAME} GLEW::glew_s)
        else()
          target_link_libraries(${NAME} GLEW::GLEW)
        endif()
      endif()
    endif()
    if(NOT ${CMAKE_VERSION} VERSION_LESS 3.9.0)
      target_link_libraries(${NAME} OpenGL::GL)
    endif()
  endif()

  if(ARG_USES_PNG)
    target_link_libraries(${NAME} PNG::PNG)
  endif()

  if(ARG_USES_SDL2)
    target_link_libraries(${NAME} ${TARGET_LINK_FLAGS} SDL2::Main ${SDL_STATIC_LIBS})
  endif()

  if(ARG_USES_SDL2_MIXER)
    target_link_libraries(${NAME} ${TARGET_LINK_FLAGS} SDL2::Mixer ${SDL_MIXER_STATIC_LIBS})
  endif()

  if(ARG_USES_SDL2_IMAGE)
    target_link_libraries(${NAME} ${TARGET_LINK_FLAGS} SDL2::Image ${SDL_IMG_STATIC_LIBS})
  endif()

  if(ARG_USES_SDL2_TTF)
    target_link_libraries(${NAME} ${TARGET_LINK_FLAGS} SDL2::TTF ${SDL_TTF_STATIC_LIBS})
  endif()

  if (ARG_USES_INTL)
    # libintl is not used on all systems, so only include it, when we actually
    # found it.
    if (Intl_FOUND)
      wl_include_system_directories(${NAME} ${Intl_INCLUDE_DIRS})
      target_link_libraries(${NAME} ${TARGET_LINK_FLAGS} ${Intl_LIBRARIES})
    endif()
  endif()

  if(ARG_USES_ICU)
    if(${CMAKE_VERSION} VERSION_LESS 3.9.0)
      wl_include_system_directories(${NAME} ${ICU_INCLUDE_DIRS})
      target_link_libraries(${NAME} ${ICU_LIBRARIES})
    else()
      target_link_libraries(${NAME} ICU::uc ICU::dt)
    endif()
  endif()

  foreach(DEPENDENCY ${ARG_DEPENDS})
    target_link_libraries(${NAME} ${TARGET_LINK_FLAGS} ${DEPENDENCY})
  endforeach(DEPENDENCY)
endmacro(_common_compile_tasks)

# Common library target definition.
function(wl_library NAME)
  _parse_common_args("${ARGN}")

  add_library(${NAME}
    STATIC
    EXCLUDE_FROM_ALL
    ${ARG_SRCS}
  )

  # increase the tries for the linker searching for cyclic dependencies
  # TODO(sirver): remove this once cycling dependencies are history in widelands
  set_target_properties(${NAME} PROPERTIES LINK_INTERFACE_MULTIPLICITY 5)

  _common_compile_tasks()
endfunction()

# Common test target definition.
function(wl_test NAME)

  if (NOT OPTION_BUILD_TESTS)
    return()
  endif()


  _parse_common_args("${ARGN}")

  add_executable(${NAME} ${ARG_SRCS})

  # Tests need to link with SDL2 library without main.
  target_link_libraries(${NAME} SDL2::Core)

  _common_compile_tasks()

  add_test(${NAME} ${NAME})
  add_dependencies(wl_tests ${NAME})
endfunction()

# Checks a single 'SRC' file using Codecheck and writes a file named
# codecheck_<shasum of input> if the codecheck did not yield anything. The
# target for the codecheck will be added as a dependency to 'NAME' for debug
# builds, but always for the target 'codecheck', so that make codecheck checks
# all source code.
function(wl_run_codecheck NAME SRC)
  get_filename_component(ABSOLUTE_SRC ${SRC} ABSOLUTE)

  # If the file does not exist, it is probably auto-generated. In that case, it
  # makes no sense to codecheck it.
  if(EXISTS ${ABSOLUTE_SRC})
    string(SHA1 CHECKSUM ${ABSOLUTE_SRC})

    set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/codecheck_${CHECKSUM}")
    add_custom_command(
      OUTPUT
        ${OUTPUT_FILE}
      COMMAND
        ${CMAKE_COMMAND}
        -DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}
        -DWL_SOURCE_CHECKER=${CMAKE_SOURCE_DIR}/cmake/codecheck/CodeCheck.py
        -DSRC=${ABSOLUTE_SRC}
        -DOUTPUT_FILE=${OUTPUT_FILE}
        -DCMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DWL_ROOT_DIR=${WL_ROOT_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/codecheck/CodeCheck.cmake
      DEPENDS ${ABSOLUTE_SRC}
      COMMENT "Checking ${SRC} with CodeCheck"
    )
    add_custom_target(
      see_if_codecheck_needs_to_run_${CHECKSUM}
      DEPENDS ${OUTPUT_FILE}
      COMMENT ""
    )

    add_dependencies(codecheck see_if_codecheck_needs_to_run_${CHECKSUM})

    if(CMAKE_BUILD_TYPE STREQUAL Debug)
      if (OPTION_BUILD_CODECHECK)
        add_dependencies(${NAME} see_if_codecheck_needs_to_run_${CHECKSUM})
      endif (OPTION_BUILD_CODECHECK)
    endif(CMAKE_BUILD_TYPE STREQUAL Debug)
  endif(EXISTS ${ABSOLUTE_SRC})
endfunction(wl_run_codecheck)

# Add a compiler flag VAR_NAME=FLAG. If VAR_NAME is already set, FLAG is appended.
function(wl_add_flag VAR_NAME FLAG)
  if (${VAR_NAME})
    set(${VAR_NAME} "${${VAR_NAME}} ${FLAG}" PARENT_SCOPE)
  else()
    set(${VAR_NAME} "${FLAG}" PARENT_SCOPE)
  endif()
endfunction()

# Common binary target definition.
function(wl_binary NAME)
  _parse_common_args("${ARGN}")

  if (ARG_WIN32)
    add_executable(${NAME}
      WIN32
      ${ARG_SRCS}
    )
  else()
    add_executable(${NAME}
      ${ARG_SRCS}
    )
  endif()

  _common_compile_tasks()

  #Quoting the CMake documentation on DESTINATION:
  #"If a relative path is given it is interpreted relative to the value of CMAKE_INSTALL_PREFIX"
  install(TARGETS ${NAME} DESTINATION "${WL_INSTALL_BINDIR}" COMPONENT ExecutableFiles)
endfunction()
