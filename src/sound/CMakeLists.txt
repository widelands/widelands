wl_library(sound
  SRCS
    ./fxset.cc
    ./fxset.h
    ./songset.cc
    ./songset.h
    #TODO: Has dependencies on graphic, logic, profile and wui... and various things in src root
    ./sound_handler.cc
    ./sound_handler.h
  DEPENDS
    io_filesystem
    base_log
)
