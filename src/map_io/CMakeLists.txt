wl_library(map_io_map_loader
  SRCS
    map_loader.h
    widelands_map_loader.cc
    widelands_map_loader.h
    s2map.cc
    s2map.h
  DEPENDS
    base_log
    base_i18n
    base_exceptions
    io_filesystem
    logic
    # TODO(sirver): These headers are not yet in separate libraries, therefore we need ball of mud.
    #include "constants.h"
    #include "scoped_timer.h"
    #include "scripting/scripting.h"
    #include "upcast.h"
    widelands_ball_of_mud
)

wl_library(map_io
  SRCS
    coords_profile.cc
    coords_profile.h
    one_world_legacy_lookup_table.cc
    one_world_legacy_lookup_table.h
    widelands_map_allowed_building_types_data_packet.cc
    widelands_map_allowed_building_types_data_packet.h
    widelands_map_allowed_worker_types_data_packet.cc
    widelands_map_allowed_worker_types_data_packet.h
    widelands_map_building_data_packet.cc
    widelands_map_building_data_packet.h
    widelands_map_buildingdata_data_packet.cc
    widelands_map_buildingdata_data_packet.h
    widelands_map_data_packet.h
    widelands_map_elemental_data_packet.cc
    widelands_map_elemental_data_packet.h
    widelands_map_exploration_data_packet.cc
    widelands_map_exploration_data_packet.h
    widelands_map_extradata_data_packet.cc
    widelands_map_extradata_data_packet.h
    widelands_map_flag_data_packet.cc
    widelands_map_flag_data_packet.h
    widelands_map_flagdata_data_packet.cc
    widelands_map_flagdata_data_packet.h
    widelands_map_heights_data_packet.cc
    widelands_map_heights_data_packet.h
    widelands_map_map_object_loader.cc
    widelands_map_map_object_loader.h
    widelands_map_map_object_saver.cc
    widelands_map_map_object_saver.h
    widelands_map_message_saver.h
    widelands_map_node_ownership_data_packet.cc
    widelands_map_node_ownership_data_packet.h
    widelands_map_object_packet.cc
    widelands_map_object_packet.h
    widelands_map_objective_data_packet.cc
    widelands_map_objective_data_packet.h
    widelands_map_player_names_and_tribes_data_packet.cc
    widelands_map_player_names_and_tribes_data_packet.h
    widelands_map_player_position_data_packet.cc
    widelands_map_player_position_data_packet.h
    widelands_map_players_messages_data_packet.cc
    widelands_map_players_messages_data_packet.h
    widelands_map_players_view_data_packet.cc
    widelands_map_players_view_data_packet.h
    widelands_map_port_spaces_data_packet.cc
    widelands_map_port_spaces_data_packet.h
    widelands_map_resources_data_packet.cc
    widelands_map_resources_data_packet.h
    widelands_map_road_data_packet.cc
    widelands_map_road_data_packet.h
    widelands_map_roaddata_data_packet.cc
    widelands_map_roaddata_data_packet.h
    widelands_map_saver.cc
    widelands_map_saver.h
    widelands_map_scripting_data_packet.cc
    widelands_map_scripting_data_packet.h
    widelands_map_terrain_data_packet.cc
    widelands_map_terrain_data_packet.h
    widelands_map_version_data_packet.cc
    widelands_map_version_data_packet.h
  USES_SDL_IMAGE
  DEPENDS
    base_log
    base_i18n
    base_exceptions
    build_info
    graphic
    io_filesystem
    logic
    wui
    # TODO(sirver): These headers are not yet in separate libraries, therefore we need ball of mud.
    #include "constants.h"
    #include "container_iterate.h"
    #include "economy/flag.h"
    #include "economy/fleet.h"
    #include "economy/portdock.h"
    #include "economy/request.h"
    #include "economy/road.h"
    #include "economy/ware_instance.h"
    #include "economy/warehousesupply.h"
    #include "economy/wares_queue.h"
    #include "helper.h"
    #include "profile/profile.h"
    #include "scoped_timer.h"
    #include "scripting/scripting.h"
    #include "upcast.h"
    widelands_ball_of_mud
)
