/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "widelands_map_saver.h"

#include "editor_game_base.h"
#include "filesystem.h"
#include "map.h"
#include "player.h"
#include "tribe.h"
#include "wexception.h"
#include "widelands_map_allowed_buildings_data_packet.h"
#include "widelands_map_bob_data_packet.h"
#include "widelands_map_bobdata_data_packet.h"
#include "widelands_map_building_data_packet.h"
#include "widelands_map_buildingdata_data_packet.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_elemental_data_packet.h"
#include "widelands_map_event_data_packet.h"
#include "widelands_map_event_chain_data_packet.h"
#include "widelands_map_extradata_data_packet.h"
#include "widelands_map_flag_data_packet.h"
#include "widelands_map_flagdata_data_packet.h"
#include "widelands_map_heights_data_packet.h"
#include "widelands_map_immovable_data_packet.h"
#include "widelands_map_immovabledata_data_packet.h"
#include "widelands_map_map_object_saver.h"
#include "widelands_map_object_packet.h"
#include "widelands_map_objective_data_packet.h"
#include "widelands_map_owned_fields_data_packet.h"
#include "widelands_map_player_names_and_tribes_data_packet.h"
#include "widelands_map_player_position_data_packet.h"
#include "widelands_map_players_areawatchers_data_packet.h"
#include "widelands_map_players_view_data_packet.h"
#include "widelands_map_resources_data_packet.h"
#include "widelands_map_road_data_packet.h"
#include "widelands_map_roaddata_data_packet.h"
#include "widelands_map_seen_fields_data_packet.h"
#include "widelands_map_terrain_data_packet.h"
#include "widelands_map_trigger_data_packet.h"
#include "widelands_map_variable_data_packet.h"
#include "widelands_map_ware_data_packet.h"
#include "widelands_map_waredata_data_packet.h"

#include "log.h"

namespace Widelands {

Map_Saver::Map_Saver(FileSystem & fs, Editor_Game_Base * egbase)
	: m_egbase(egbase), m_fs(fs), m_mos(0)
{}


Map_Saver::~Map_Saver() {delete m_mos;}


void Map_Saver::save() throw (_wexception) {
	delete m_mos;
	m_mos = new Map_Map_Object_Saver();

	// The binary data is saved in an own directory
	// to keep it hidden from the poor debuggers
	m_fs.EnsureDirectoryExists("binary");

	// MANDATORY PACKETS
	// Start with writing the map out, first Elemental data
	// PRELOAD DATA BEGIN
	log("Writing Elemental Data ... ");
	{Map_Elemental_Data_Packet               p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");


	log("Writing Player Names And Tribe Data ... ");
	{Map_Player_Names_And_Tribes_Data_Packet p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");
	//  PRELOAD DATA END

	log("Writing Heights Data ... ");
	{Map_Heights_Data_Packet                 p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	log("Writing Terrain Data ... ");
	{Map_Terrain_Data_Packet                 p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	log("Writing Player Start Position Data ... ");
	{Map_Player_Position_Data_Packet         p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	log("Writing Bob Data ... ");
	{Map_Bob_Data_Packet                     p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	log("Writing Resources Data ... ");
	{Map_Resources_Data_Packet               p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	//  NON MANDATORY PACKETS BELOW THIS POINT
	log("Writing Map Extra Data ... ");
	{Map_Extradata_Data_Packet               p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	const Map & map = *m_egbase->get_map();

	if (map.mtm().size()) {
		log("Writing Trigger Data ... ");
		Map_Trigger_Data_Packet               p; p.Write(m_fs, m_egbase, m_mos);
		log("done!\n ");
	}

	if (map.mem().size()) {
		log("Writing Event Data ... ");
		Map_Event_Data_Packet                 p; p.Write(m_fs, m_egbase, m_mos);
		log("done!\n ");
	}

	if (map.mcm().size()) {
		log("Writing Event Chain Data ... ");
		Map_EventChain_Data_Packet            p; p.Write(m_fs, m_egbase, m_mos);
		log("done!\n ");
	}

	//  allowed buildings
	const Player_Number nr_players = map.get_nrplayers();
	iterate_players_existing_const(plnum, nr_players, *m_egbase, player) {
		Building_Index const nr_buildings = player->tribe().get_nrbuildings();
		for (Building_Index i = Building_Index::First(); i < nr_buildings; ++i)
			if (not player->is_building_allowed(i)) {
				log("Writing Allowed Buildings Data ... ");
				Map_Allowed_Buildings_Data_Packet p;
				p.Write(m_fs, m_egbase, m_mos);
				log("done!\n ");
				goto end_outer_loop;
			}
	} end_outer_loop:;

	// !!!!!!!!!! NOTE
	// This packet must be before any building or road packet. So do not
	// change this order without knowing what you do
	// EXISTENT PACKETS
	log("Writing Flag Data ... ");
	{Map_Flag_Data_Packet                    p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	log("Writing Road Data ... ");
	{Map_Road_Data_Packet                    p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	log("Writing Building Data ... ");
	{Map_Building_Data_Packet                p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");


	log("Writing Map Ware Data ... ");
	{Map_Ware_Data_Packet                    p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	log("Writing Map Objects ... ");
	{Map_Object_Packet                       p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	// DATA PACKETS
	if (m_mos->get_nr_flags()) {
		log("Writing Flagdata Data ... ");
		{Map_Flagdata_Data_Packet             p; p.Write(m_fs, m_egbase, m_mos);}
		log("done!\n ");
	}

	if (m_mos->get_nr_roads()) {
		log("Writing Roaddata Data ... ");
		{Map_Roaddata_Data_Packet             p; p.Write(m_fs, m_egbase, m_mos);}
		log("done!\n ");
	}


	if (m_mos->get_nr_buildings()) {
		log("Writing Buildingdata Data ... ");
		{Map_Buildingdata_Data_Packet         p; p.Write(m_fs, m_egbase, m_mos);}
		log("done!\n ");
	}


	if (m_mos->get_nr_wares()) {
		log("Writing Waredata Data ... ");
		{Map_Waredata_Data_Packet             p; p.Write(m_fs, m_egbase, m_mos);}
		log("done!\n ");
	}

	if (m_mos->get_nr_bobs()) {
		log("Writing Bobdata Data ... ");
		{Map_Bobdata_Data_Packet              p; p.Write(m_fs, m_egbase, m_mos);}
		log("done!\n ");
	}

	log("Writing Owned-Fields Data ... ");
	{Map_Owned_Fields_Data_Packet            p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	log("Writing Seen-Fields Data ... ");
	{Map_Seen_Fields_Data_Packet             p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	log("Writing Area Watchers Data ... ");
	{Map_Players_AreaWatchers_Data_Packet    p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	log("Writing Players Unseen Data ... ");
	{Map_Players_View_Data_Packet            p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	log("Writing Variable Data ... ");
	{Map_Variable_Data_Packet                p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	log("Writing Objective Data ... ");
	{Map_Objective_Data_Packet               p; p.Write(m_fs, m_egbase, m_mos);}
	log("done!\n ");

	if (m_mos->get_nr_unsaved_objects())
		throw wexception
			("There are %i unsaved objects. This is a bug, please consider "
			 "committing!",
			 m_mos->get_nr_unsaved_objects());
}

};
