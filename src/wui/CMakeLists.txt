wl_library(wui
  SRCS
    actionconfirm.cc
    actionconfirm.h
    attack_box.cc
    attack_box.h
    building_statistics_menu.cc
    building_statistics_menu.h
    building_ui.cc
    buildingwindow.cc
    buildingwindow.h
    chatoverlay.cc
    chatoverlay.h
    CMakeLists.txt
    constructionsitewindow.cc
    debugconsole.cc
    debugconsole.h
    dismantlesitewindow.cc
    encyclopedia_window.cc
    encyclopedia_window.h
    fieldaction.cc
    fieldaction.h
    game_chat_menu.cc
    game_chat_menu.h
    game_debug_ui.cc
    game_debug_ui.h
    game_main_menu.cc
    game_main_menu.h
    game_main_menu_save_game.cc
    game_main_menu_save_game.h
    game_message_menu.cc
    game_message_menu.h
    game_objectives_menu.cc
    game_objectives_menu.h
    game_options_menu.cc
    game_options_menu.h
    game_options_sound_menu.cc
    game_options_sound_menu.h
    game_summary.cc
    game_summary.h
    game_tips.cc
    game_tips.h
    gamechatpanel.cc
    gamechatpanel.h
    general_statistics_menu.cc
    general_statistics_menu.h
    interactive_base.cc
    interactive_base.h
    interactive_gamebase.cc
    interactive_gamebase.h
    interactive_player.cc
    interactive_player.h
    interactive_spectator.cc
    interactive_spectator.h
    itemwaresdisplay.cc
    itemwaresdisplay.h
    login_box.cc
    login_box.h
    logmessage.h
    mapview.cc
    mapview.h
    mapviewpixelconstants.h
    mapviewpixelfunctions.cc
    mapviewpixelfunctions.h
    military_box.cc
    military_box.h
    militarysitewindow.cc
    minimap.cc
    minimap.h
    multiplayersetupgroup.cc
    multiplayersetupgroup.h
    overlay_manager.cc
    overlay_manager.h
    playerdescrgroup.cc
    playerdescrgroup.h
    plot_area.cc
    plot_area.h
    portdockwaresdisplay.cc
    portdockwaresdisplay.h
    productionsitewindow.cc
    productionsitewindow.h
    quicknavigation.cc
    quicknavigation.h
    shipwindow.cc
    soldiercapacitycontrol.cc
    soldiercapacitycontrol.h
    soldierlist.cc
    soldierlist.h
    stock_menu.cc
    stock_menu.h
    story_message_box.cc
    story_message_box.h
    trainingsitewindow.cc
    transport_draw.cc
    transport_ui.cc
    unique_window_handler.cc
    unique_window_handler.h
    vector.h
    ware_statistics_menu.cc
    ware_statistics_menu.h
    warehousewindow.cc
    waresdisplay.cc
    waresdisplay.h
    waresqueuedisplay.cc
    waresqueuedisplay.h
    watchwindow.cc
    watchwindow.h
  DEPENDS
    base_log
    base_i18n
    base_exceptions
    graphic
    io_filesystem
    logic
    network
    profile
    sound
    ui_basic
    # TODO(sirver): These headers are not yet in separate libraries, therefore we need ball of mud.
    #include "chat.h"
    #include "constants.h"
    #include "container_iterate.h"
    #include "economy/economy.h"
    #include "economy/flag.h"
    #include "economy/portdock.h"
    #include "economy/request.h"
    #include "economy/road.h"
    #include "economy/ware_instance.h"
    #include "economy/wares_queue.h"
    #include "game_io/game_loader.h"
    #include "game_io/game_preload_data_packet.h"
    #include "game_io/game_saver.h"
    #include "gamecontroller.h"
    #include "gamesettings.h"
    #include "helper.h"
    #include "point.h"
    #include "ref_cast.h"
    #include "rgbcolor.h"
    #include "scripting/lua_table.h"
    #include "scripting/scripting.h"
    #include "text_layout.h"
    #include "timestring.h"
    #include "ui_fsmenu/fileview.h"
    #include "upcast.h"
    #include "wlapplication.h"
    widelands_ball_of_mud
)
