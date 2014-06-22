wl_library(base_port
  SRCS
    port.h
    port.cc
)

wl_library(base_log
  SRCS
    log.cc
    log.h
)

wl_library(base_exceptions
  SRCS
    exceptions.cc
    warning.h
    wexception.h
  DEPENDS
    base_port
)

wl_library(base_i18n
  SRCS
    i18n.h
    i18n.cc
  USES_INTL
  DEPENDS
    base_log
    base_port
)
