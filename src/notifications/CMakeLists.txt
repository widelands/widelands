add_subdirectory(test)

wl_library(notifications
  SRCS
    notifications.cc
    notifications.h
  DEPENDS
    base_log
)
