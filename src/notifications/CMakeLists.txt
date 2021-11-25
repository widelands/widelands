add_subdirectory(test)

wl_library(notifications
  SRCS
    notifications.cc
    notifications.h
    notifications_impl.h
    note_ids.h
    signal.h
  DEPENDS
    base_exceptions
    base_log
    base_macros
)
