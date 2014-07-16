add_subdirectory(test)

wl_library(notifications
  SRCS
    notifications.cc
    notifications.h
    notifications_impl.h
    note_ids.h
  DEPENDS
    base_log
    base_macros
)
