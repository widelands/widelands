add_subdirectory(test)

wl_library(base
  SRCS
    format_impl.h
    i18n.h
    i18n.cc
    multithreading.h
    multithreading.cc
    log.cc
    log.h
    string.h
    string.cc
  USES_ATOMIC
  USES_INTL
  USES_SDL2
  DEPENDS
    base_exceptions
    base_macros
    base_times
    build_info
    io_filesystem
    logic_filesystem_constants
    notifications
    third_party_gettext
)

wl_library(base_macros
  SRCS
    macros.h
    macros.cc
)

wl_library(base_exceptions
  SRCS
    exceptions.cc
    warning.h
    wexception.h
  DEPENDS
    base_macros
)

wl_library(base_utf8
  SRCS
    utf8.h
    utf8.cc
)

wl_library(base_geometry
  SRCS
    rect.h
    vector.h
    vector.cc
)

wl_library(base_md5
  SRCS
    md5.cc
    md5.h
  DEPENDS
    base_macros
)

wl_library(base_random
  SRCS
    random.h
    random.cc
  DEPENDS
    io_stream
    base_exceptions
)


wl_library(base_scoped_timer
  SRCS
    scoped_timer.h
    scoped_timer.cc
  USES_SDL2
  DEPENDS
    base
    base_macros
)


wl_library(base_time_string
  SRCS
    time_string.h
    time_string.cc
  DEPENDS
    base
)


wl_library(base_times
  SRCS
    times.h
    times.cc
  USES_ATOMIC
  DEPENDS
    base_exceptions
    io_fileread
)


wl_library(base_math
  SRCS
    math.h
    math.cc
  DEPENDS
    base_exceptions
)

wl_library(base_test
  SRCS
    test.h
    test.cc
  DEPENDS
    base
    base_exceptions
)
