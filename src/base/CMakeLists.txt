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
)

wl_library(base_i18n
  SRCS
    i18n.h
    i18n.cc
  DEPENDS
    base_log
    ${INTL_LIBRARY}
)
