wl_library(base_macros
  SRCS
    macros.h
    macros.cc
)

wl_library(base_log
  SRCS
    log.cc
    log.h
  USES_SDL2
  DEPENDS
    base_macros
    base_exceptions
)

wl_library(base_exceptions
  SRCS
    exceptions.cc
    warning.h
    wexception.h
  DEPENDS
    base_macros
)

wl_library(base_i18n
  SRCS
    i18n.h
    i18n.cc
    utf8.h
  USES_INTL
  DEPENDS
    base_log
    base_macros
    third_party_gettext
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

wl_library(base_scoped_timer
  SRCS
    scoped_timer.h
    scoped_timer.cc
  USES_SDL2
  DEPENDS
    base_log
    base_macros
)

wl_library(base_time_string
  SRCS
    time_string.h
    time_string.cc
  DEPENDS
    base_i18n
)

wl_library(base_math
  SRCS
    math.h
    math.cc
  DEPENDS
    base_macros
)
