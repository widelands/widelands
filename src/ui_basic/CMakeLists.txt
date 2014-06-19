
wl_library(ui_basic
  SRCS
    box.cc
    box.h
    button.cc
    button.h
    checkbox.cc
    checkbox.h
    editbox.cc
    editbox.h
    helpwindow.cc
    helpwindow.h
    icon.cc
    icon.h
    icongrid.cc
    icongrid.h
    listselect.cc
    listselect.h
    messagebox.cc
    messagebox.h
    mouse_constants.h
    multilineeditbox.cc
    multilineeditbox.h
    multilinetextarea.cc
    multilinetextarea.h
    panel.cc
    panel.h
    progressbar.cc
    progressbar.h
    progresswindow.cc
    progresswindow.h
    radiobutton.cc
    radiobutton.h
    scrollbar.cc
    scrollbar.h
    slider.cc
    slider.h
    spinbox.cc
    spinbox.h
    table.cc
    table.h
    tabpanel.cc
    tabpanel.h
    textarea.cc
    textarea.h
    unique_window.cc
    unique_window.h
    window.cc
    window.h
  USES_BOOST_REGEX
  DEPENDS
    base_exceptions
    base_i18n
    base_log
    graphic
    io_filesystem
    profile
    sound
    # TODO(sirver): These headers are not yet in separate libraries, therefore we need ball of mud.
    #include "constants.h"
    #include "container_iterate.h"
    #include "helper.h"
    #include "point.h"
    #include "rgbcolor.h"
    #include "scripting/lua_table.h"
    #include "scripting/scripting.h"
    #include "text_layout.h"
    #include "utf8.h"
    #include "wlapplication.h"
    widelands_ball_of_mud
)
