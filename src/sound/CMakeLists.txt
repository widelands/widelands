wl_library(sound
  SRCS
    ./fxset.cc
    ./fxset.h
    ./songset.cc
    ./songset.h
    #TODO: Has dependencies on graphic, logic, and wui... and various things in src root
    ./sound_handler.cc
    ./sound_handler.h
  USES_BOOST_REGEX
  DEPENDS
    base_log
    io_filesystem
    profile
)
