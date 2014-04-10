/*
 * Copyright (C) 2002-2004, 2006-2011, 2013 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "map_io/widelands_map_saver.h"

#include "log.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "map_io/widelands_map_allowed_building_types_data_packet.h"
#include "map_io/widelands_map_allowed_worker_types_data_packet.h"
#include "map_io/widelands_map_building_data_packet.h"
#include "map_io/widelands_map_buildingdata_data_packet.h"
#include "map_io/widelands_map_elemental_data_packet.h"
#include "map_io/widelands_map_exploration_data_packet.h"
#include "map_io/widelands_map_extradata_data_packet.h"
#include "map_io/widelands_map_flag_data_packet.h"
#include "map_io/widelands_map_flagdata_data_packet.h"
#include "map_io/widelands_map_heights_data_packet.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "map_io/widelands_map_node_ownership_data_packet.h"
#include "map_io/widelands_map_object_packet.h"
#include "map_io/widelands_map_objective_data_packet.h"
#include "map_io/widelands_map_player_names_and_tribes_data_packet.h"
#include "map_io/widelands_map_player_position_data_packet.h"
#include "map_io/widelands_map_players_messages_data_packet.h"
#include "map_io/widelands_map_players_view_data_packet.h"
#include "map_io/widelands_map_port_spaces_data_packet.h"
#include "map_io/widelands_map_resources_data_packet.h"
#include "map_io/widelands_map_road_data_packet.h"
#include "map_io/widelands_map_roaddata_data_packet.h"
#include "map_io/widelands_map_scripting_data_packet.h"
#include "map_io/widelands_map_terrain_data_packet.h"
#include "map_io/widelands_map_version_data_packet.h"
#include "scoped_timer.h"
#include "wexception.h"

namespace Widelands {

Map_Saver::Map_Saver(FileSystem & fs, Editor_Game_Base & egbase)
	: m_egbase(egbase), m_fs(fs), m_mos(nullptr)
{}


Map_Saver::~Map_Saver() {delete m_mos;}


void Map_Saver::save() {
	ScopedTimer timer("Map_Saver::save() took %ums");

	delete m_mos;
	m_mos = new Map_Map_Object_Saver();

	// The binary data is saved in an own directory
	// to keep it hidden from the poor debuggers
	m_fs.EnsureDirectoryExists("binary");

	// MANDATORY PACKETS
	// Start with writing the map out, first Elemental data
	// PRELOAD DATA BEGIN
	log("Writing Elemental Data ... ");
	{Map_Elemental_Data_Packet               p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Player Names And Tribe Data ... ");
	{Map_Player_Names_And_Tribes_Data_Packet p; p.Write(m_fs, m_egbase, *m_mos);
	}
	log("took %ums\n ", timer.ms_since_last_query());
	//  PRELOAD DATA END

	log("Writing Port Spaces Data ... ");
	{Map_Port_Spaces_Data_Packet             p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Heights Data ... ");
	{Map_Heights_Data_Packet                 p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Terrain Data ... ");
	{Map_Terrain_Data_Packet                 p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Player Start Position Data ... ");
	{Map_Player_Position_Data_Packet         p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	//  This must come before anything that references messages, such as:
	//    * command queue (PlayerMessageCommand, inherited by
	//      Cmd_MessageSetStatusRead and Cmd_MessageSetStatusArchived)
	log("Writing Player Message Data ... ");
	{Map_Players_Messages_Data_Packet        p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Resources Data ... ");
	{Map_Resources_Data_Packet               p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	//  NON MANDATORY PACKETS BELOW THIS POINT
	log("Writing Map Extra Data ... ");
	{Map_Extradata_Data_Packet               p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Map Version ... ");
	{Map_Version_Data_Packet               p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());



	const Map & map = m_egbase.map();

	Player_Number const nr_players = map.get_nrplayers();

	//  allowed worker types
	log("Writing Allowed Worker Types Data ... ");
	{Map_Allowed_Worker_Types_Data_Packet p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

 //  allowed building types
	iterate_players_existing_const(plnum, nr_players, m_egbase, player) {
		Building_Index const nr_buildings = player->tribe().get_nrbuildings();
		for (Building_Index i = Building_Index::First(); i < nr_buildings; ++i)
			if (not player->is_building_type_allowed(i)) {
				log("Writing Allowed Building Types Data ... ");
				Map_Allowed_Building_Types_Data_Packet p;
				p                                  .Write(m_fs, m_egbase, *m_mos);
				log("took %ums\n ", timer.ms_since_last_query());
				goto end_find_a_forbidden_building_type_loop;
			}
	} end_find_a_forbidden_building_type_loop:;

	// !!!!!!!!!! NOTE
	// This packet must be before any building or road packet. So do not
	// change this order without knowing what you do
	// EXISTENT PACKETS
	log("Writing Flag Data ... ");
	{Map_Flag_Data_Packet                   p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Road Data ... ");
	{Map_Road_Data_Packet                   p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Building Data ... ");
	{Map_Building_Data_Packet               p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Map Objects ... ");
	{Map_Object_Packet                      p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	// DATA PACKETS
	if (m_mos->get_nr_flags()) {
		log("Writing Flagdata Data ... ");
		{Map_Flagdata_Data_Packet            p; p.Write(m_fs, m_egbase, *m_mos);}
		log("took %ums\n ", timer.ms_since_last_query());
	}

	if (m_mos->get_nr_roads()) {
		log("Writing Roaddata Data ... ");
		{Map_Roaddata_Data_Packet            p; p.Write(m_fs, m_egbase, *m_mos);}
		log("took %ums\n ", timer.ms_since_last_query());
	}

	if (m_mos->get_nr_buildings()) {
		log("Writing Buildingdata Data ... ");
		{Map_Buildingdata_Data_Packet        p; p.Write(m_fs, m_egbase, *m_mos);}
		log("took %ums\n ", timer.ms_since_last_query());
	}

	log("Writing Node Ownership Data ... ");
	{Map_Node_Ownership_Data_Packet         p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Exploration Data ... ");
	{Map_Exploration_Data_Packet            p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Players Unseen Data ... ");
	{Map_Players_View_Data_Packet           p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Scripting Data ... ");
	{Map_Scripting_Data_Packet              p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Objective Data ... ");
	{Map_Objective_Data_Packet              p; p.Write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

#ifndef NDEBUG
	m_mos->detect_unsaved_objects();
#endif
}

}
