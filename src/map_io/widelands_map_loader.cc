/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#include "widelands_map_loader.h"

#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "warning.h"
#include "widelands_map_allowed_building_types_data_packet.h"
#include "widelands_map_allowed_worker_types_data_packet.h"
#include "widelands_map_bob_data_packet.h"
#include "widelands_map_bobdata_data_packet.h"
#include "widelands_map_building_data_packet.h"
#include "widelands_map_buildingdata_data_packet.h"
#include "widelands_map_elemental_data_packet.h"
#include "widelands_map_event_data_packet.h"
#include "widelands_map_event_chain_data_packet.h"
#include "widelands_map_exploration_data_packet.h"
#include "widelands_map_extradata_data_packet.h"
#include "widelands_map_flag_data_packet.h"
#include "widelands_map_flagdata_data_packet.h"
#include "widelands_map_heights_data_packet.h"
#include "widelands_map_immovable_data_packet.h"
#include "widelands_map_immovabledata_data_packet.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_node_ownership_data_packet.h"
#include "widelands_map_object_packet.h"
#include "widelands_map_objective_data_packet.h"
#include "widelands_map_player_names_and_tribes_data_packet.h"
#include "widelands_map_player_position_data_packet.h"
#include "widelands_map_players_areawatchers_data_packet.h"
#include "widelands_map_players_view_data_packet.h"
#include "widelands_map_resources_data_packet.h"
#include "widelands_map_road_data_packet.h"
#include "widelands_map_roaddata_data_packet.h"
#include "widelands_map_terrain_data_packet.h"
#include "widelands_map_trigger_data_packet.h"
#include "widelands_map_variable_data_packet.h"
#include "widelands_map_ware_data_packet.h"
#include "widelands_map_waredata_data_packet.h"
#include "logic/world.h"

#include "log.h"

namespace Widelands {

WL_Map_Loader::WL_Map_Loader(FileSystem & fs, Map * const m)
	: Map_Loader("", *m), m_fs(fs), m_mol(0)
{}


WL_Map_Loader::~WL_Map_Loader() {
	delete m_mol;
	delete &m_fs;
}

/**
 * Preloads a map so that the map class returns valid data for all it's
 * get_info() functions (_width, _nrplayers..)
 */
int32_t WL_Map_Loader::preload_map(bool const scenario) {
	assert(get_state() != STATE_LOADED);

	m_map.cleanup();

	{Map_Elemental_Data_Packet mp; mp.Pre_Read(m_fs, &m_map);}

	if (not World::exists_world(m_map.get_world_name()))
		throw warning
			(_("Invalid World"),
			 _
			 	("The world \"%s\" set by the map, does not exist on your "
			 	 "filesystem."),
			 m_map.get_world_name());

	{
		Map_Player_Names_And_Tribes_Data_Packet p;
		p.Pre_Read(m_fs, &m_map, !scenario);
	}
	// No trigger file -> not playable as scenario!
	m_map.set_as_scenario_playable(m_fs.FileExists("trigger"));

	set_state(STATE_PRELOADED);

	return 0;
}


void WL_Map_Loader::load_world() {
	assert(get_state() == STATE_PRELOADED);
	m_map.load_world();
	set_state(STATE_WORLD_LOADED);
}


/*
 * Load the complete map and make sure that it runs without problems
 */
int32_t WL_Map_Loader::load_map_complete
	(Editor_Game_Base & egbase, bool const scenario)
{
	//  This is needed to ensure that world is loaded for multiplayer clients,
	//  hosts do world loading while creating the game and the states are not
	//  available outside this class to make a conditional load. If You know a
	//  better way to fix this, DO IT! -- Alexia Death
	if (get_state() == STATE_PRELOADED)
		load_world();
	assert(get_state() == STATE_WORLD_LOADED);

	m_map.set_size(m_map.m_width, m_map.m_height);

	delete m_mol;
	m_mol = new Map_Map_Object_Loader;

	// MANDATORY PACKETS
	// Start with writing the map out, first Elemental data
	// PRELOAD DATA BEGIN
	log("Reading Elemental Data ... ");
	{Map_Elemental_Data_Packet       p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	egbase.allocate_player_maps(); //  Can do this now that map size is known.

	//  now player names and tribes
	log("Reading Player Names And Tribe Data ... ");
	{
		Map_Player_Names_And_Tribes_Data_Packet p;
		p.Read(m_fs, egbase, !scenario, m_mol);
	}
	log("done!\n ");
	// PRELOAD DATA END

	log("Reading Heights Data ... ");
	{Map_Heights_Data_Packet         p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	log("Reading Terrain Data ... ");
	{Map_Terrain_Data_Packet         p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	Map_Object_Packet mapobjects;

	if (bool const have_immovables = m_fs.FileExists("binary/immovable")) {
		log("Reading Immovable Data ... ");
		{Map_Immovable_Data_Packet p;    p.Read(m_fs, egbase, !scenario, m_mol);}
		log("done!\n ");

		if (m_fs.FileExists("binary/mapobjects"))
			log("Warning: Map has both binary/immovable and binary/mapobjects\n");
	} else {
		log("Reading Map Objects ... ");
		mapobjects.Read(m_fs, egbase, m_mol);
		log("done\n");
	}

	log("Reading Player Start Position Data ... ");
	{Map_Player_Position_Data_Packet p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	log("Reading Bob Data ... ");
	{Map_Bob_Data_Packet             p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	log("Reading Resources Data ... ");
	{Map_Resources_Data_Packet       p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	//  NON MANDATORY PACKETS BELOW THIS POINT
	log("Reading Map Extra Data ... ");
	{Map_Extradata_Data_Packet       p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	log("Reading Trigger Data ... ");
	{Map_Trigger_Data_Packet         p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	//  Objectives: Depends on triggers
	log("Reading Objective Data ... ");
	{Map_Objective_Data_Packet       p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	//  Events: depends on triggers, objectives
	log("Reading Event Data ... ");
	{Map_Event_Data_Packet           p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	//  Event Chains: depends on events, triggers
	log("Reading Event Chain Data ... ");
	{Map_EventChain_Data_Packet      p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	log("Reading Allowed Worker Types Data ... ");
	{
		Map_Allowed_Worker_Types_Data_Packet p;
		p.Read(m_fs, egbase, !scenario, m_mol);
	}
	log("done!\n ");

	log("Reading Allowed Building Types Data ... ");
	{
		Map_Allowed_Building_Types_Data_Packet p;
		p.Read(m_fs, egbase, !scenario, m_mol);
	}
	log("done!\n ");

	log("Reading Node Ownership Data ... ");
	{Map_Node_Ownership_Data_Packet  p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	log("Reading Exploration Data ... ");
	{Map_Exploration_Data_Packet     p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	log("Reading AreaWatchers Data ... ");
	{
		Map_Players_AreaWatchers_Data_Packet p;
		p.Read(m_fs, egbase, !scenario, m_mol);
	}
	log("done!\n ");

	//  We always write the next few packets since it takes too much time
	//  looking if it really is needed.

	//  !!!!!!!!!! NOTE
	//  This packet must be before any building or road packet. So do not change
	//  this order without knowing what you do
	//  EXISTENT PACKETS
	log("Reading Flag Data ... ");
	{Map_Flag_Data_Packet            p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	log("Reading Road Data ... ");
	{Map_Road_Data_Packet            p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	log("Reading Building Data ... ");
	{Map_Building_Data_Packet        p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");


	log("Reading Map Ware Data ... ");
	{Map_Ware_Data_Packet            p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	//  DATA PACKETS
	log("Reading Flagdata Data ... ");
	{Map_Flagdata_Data_Packet        p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	log("Reading Roaddata Data ... ");
	{Map_Roaddata_Data_Packet        p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");


	log("Reading Buildingdata Data ... ");
	{Map_Buildingdata_Data_Packet    p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");


	log("Reading Waredata Data ... ");
	{Map_Waredata_Data_Packet        p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	log("Reading Bobdata Data ... ");
	{Map_Bobdata_Data_Packet         p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	log("Reading Immovabledata Data ... ");
	//  We do this only for binary compatibility.
	{Map_Immovabledata_Data_Packet   p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	log("Second and third phase loading Map Objects ... ");
	mapobjects.LoadFinish();
	{
		Field const & fields_end = map()[map().max_index()];
		for (Field * field = &map()[0]; field < &fields_end; ++field)
			if (BaseImmovable * const imm = field->get_immovable()) {
				if (upcast(Building const, building, imm))
					if (field != &map()[building->get_position()])
						continue; //  not the building's main position
				imm->load_finish(egbase);
			}
	}
	log("done\n");

	//  This should be at least after loading Soldiers (Bobs).
	//  NOTE DO NOT CHANGE THE PLACE UNLESS YOU KNOW WHAT ARE YOU DOING
	//  Must be loaded after every kind of object that can see.
	log("Reading Players View Data ... ");
	{Map_Players_View_Data_Packet    p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	log("Reading Variable Data ... ");
	{Map_Variable_Data_Packet        p; p.Read(m_fs, egbase, !scenario, m_mol);}
	log("done!\n ");

	if (m_mol->get_nr_unloaded_objects())
		log
			("WARNING: There are %i unloaded objects. This is a bug, please "
			 "consider committing!\n",
			 m_mol->get_nr_unloaded_objects());

	m_map.recalc_whole_map();

	set_state(STATE_LOADED);

	return 0;
}

}
