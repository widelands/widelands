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
    #TODO: (code review): Oh, and another thing; do we want to specificy dependencies for SDL/Boost/et al for each library? Does this make dependencies clearer and explicit or is it overkill?
)
