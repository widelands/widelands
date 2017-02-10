/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "map_io/widelands_map_loader.h"

#include <memory>

#include "base/log.h"
#include "base/scoped_timer.h"
#include "base/warning.h"
#include "io/filesystem/filesystem.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "map_io/map_allowed_building_types_packet.h"
#include "map_io/map_allowed_worker_types_packet.h"
#include "map_io/map_bob_packet.h"
#include "map_io/map_building_packet.h"
#include "map_io/map_buildingdata_packet.h"
#include "map_io/map_elemental_packet.h"
#include "map_io/map_exploration_packet.h"
#include "map_io/map_flag_packet.h"
#include "map_io/map_flagdata_packet.h"
#include "map_io/map_heights_packet.h"
#include "map_io/map_images.h"
#include "map_io/map_node_ownership_packet.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_packet.h"
#include "map_io/map_objective_packet.h"
#include "map_io/map_player_names_and_tribes_packet.h"
#include "map_io/map_player_position_packet.h"
#include "map_io/map_players_messages_packet.h"
#include "map_io/map_players_view_packet.h"
#include "map_io/map_port_spaces_packet.h"
#include "map_io/map_resources_packet.h"
#include "map_io/map_road_packet.h"
#include "map_io/map_roaddata_packet.h"
#include "map_io/map_scripting_packet.h"
#include "map_io/map_terrain_packet.h"
#include "map_io/map_version_packet.h"
#include "map_io/tribes_legacy_lookup_table.h"
#include "map_io/world_legacy_lookup_table.h"

namespace Widelands {

WidelandsMapLoader::WidelandsMapLoader(FileSystem* fs, Map* const m) : MapLoader("", *m), fs_(fs) {
	m->filesystem_.reset(fs);
}

WidelandsMapLoader::~WidelandsMapLoader() {
}

/**
 * Preloads a map so that the map class returns valid data for all it's
 * get_info() functions (width, nrplayers..)
 */
int32_t WidelandsMapLoader::preload_map(bool const scenario) {
	assert(get_state() != STATE_LOADED);

	map_.cleanup();

	{
		MapElementalPacket mp;
		mp.pre_read(*fs_, &map_);
		old_world_name_ = mp.old_world_name();
	}

	{
		MapPlayerNamesAndTribesPacket p;
		p.pre_read(*fs_, &map_, !scenario);
	}
	// No scripting/init.lua file -> not playable as scenario
	Map::ScenarioTypes m = Map::NO_SCENARIO;
	if (fs_->file_exists("scripting/init.lua"))
		m |= Map::SP_SCENARIO;
	if (fs_->file_exists("scripting/multiplayer_init.lua"))
		m |= Map::MP_SCENARIO;
	map_.set_scenario_types(m);

	set_state(STATE_PRELOADED);

	return 0;
}

/*
 * Load the complete map and make sure that it runs without problems
 */
int32_t WidelandsMapLoader::load_map_complete(EditorGameBase& egbase,
                                              MapLoader::LoadType load_type) {
	std::string timer_message = "WidelandsMapLoader::load_map_complete() for '";
	timer_message += map_.get_name();
	timer_message += "' took %ums";
	ScopedTimer timer(timer_message);

	bool is_game = load_type == MapLoader::LoadType::kGame;

	preload_map(!is_game);
	map_.set_size(map_.width_, map_.height_);
	mol_.reset(new MapObjectLoader());

	// MANDATORY PACKETS
	// PRELOAD DATA BEGIN
	log("Reading Elemental Data ... ");
	MapElementalPacket elemental_data_packet;
	elemental_data_packet.read(*fs_, egbase, is_game, *mol_);
	log("took %ums\n ", timer.ms_since_last_query());

	egbase.allocate_player_maps();  //  Can do this now that map size is known.

	//  now player names and tribes
	log("Reading Player Names And Tribe Data ... ");
	{
		MapPlayerNamesAndTribesPacket p;
		p.read(*fs_, egbase, is_game, *mol_);
	}
	log("took %ums\n ", timer.ms_since_last_query());
	// PRELOAD DATA END

	if (fs_->file_exists("port_spaces")) {
		log("Reading Port Spaces Data ... ");

		MapPortSpacesPacket p;
		p.read(*fs_, egbase, is_game, *mol_);

		log("took %ums\n ", timer.ms_since_last_query());
	}

	log("Reading Heights Data ... ");
	{
		MapHeightsPacket p;
		p.read(*fs_, egbase, is_game, *mol_);
	}
	log("took %ums\n ", timer.ms_since_last_query());

	std::unique_ptr<WorldLegacyLookupTable> world_lookup_table(
	   create_world_legacy_lookup_table(old_world_name_));
	std::unique_ptr<TribesLegacyLookupTable> tribe_lookup_table(new TribesLegacyLookupTable());
	log("Reading Terrain Data ... ");
	{
		MapTerrainPacket p;
		p.read(*fs_, egbase, *world_lookup_table);
	}
	log("took %ums\n ", timer.ms_since_last_query());

	MapObjectPacket mapobjects;

	log("Reading Map Objects ... ");
	mapobjects.read(*fs_, egbase, *mol_, *world_lookup_table, *tribe_lookup_table);
	log("took %ums\n ", timer.ms_since_last_query());

	log("Reading Player Start Position Data ... ");
	{
		MapPlayerPositionPacket p;
		p.read(*fs_, egbase, is_game, *mol_);
	}
	log("took %ums\n ", timer.ms_since_last_query());

	// This call must stay around forever since this was the way critters have
	// been saved into the map before 2010. Most of the maps we ship are still
	// old in that sense and most maps on the homepage too.
	if (fs_->file_exists("binary/bob")) {
		log("Reading (legacy) Bob Data ... ");
		{
			MapBobPacket p;
			p.read(*fs_, egbase, *mol_, *world_lookup_table);
		}
		log("took %ums\n ", timer.ms_since_last_query());
	}

	log("Reading Resources Data ... ");
	{
		MapResourcesPacket p;
		p.read(*fs_, egbase, *world_lookup_table);
	}
	log("took %ums\n ", timer.ms_since_last_query());

	//  NON MANDATORY PACKETS BELOW THIS POINT
	// Do not load unneeded packages in the editor
	if (load_type != MapLoader::LoadType::kEditor) {
		log("Reading Map Version Data ... ");
		{
			MapVersionPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		log("Reading Allowed Worker Types Data ... ");
		{
			MapAllowedWorkerTypesPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		log("Reading Allowed Building Types Data ... ");
		{
			MapAllowedBuildingTypesPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		log("Reading Node Ownership Data ... ");
		{
			MapNodeOwnershipPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		log("Reading Exploration Data ... ");
		{
			MapExplorationPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		//  !!!!!!!!!! NOTE
		//  This packet must be before any building or road packet. So do not change
		//  this order without knowing what you do
		//  EXISTENT PACKETS
		log("Reading Flag Data ... ");
		{
			MapFlagPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		log("Reading Road Data ... ");
		{
			MapRoadPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		log("Reading Building Data ... ");
		{
			MapBuildingPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		//  DATA PACKETS
		log("Reading Flagdata Data ... ");
		{
			MapFlagdataPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		log("Reading Roaddata Data ... ");
		{
			MapRoaddataPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		log("Reading Buildingdata Data ... ");
		{
			MapBuildingdataPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		log("Second and third phase loading Map Objects ... ");
		mapobjects.load_finish();
		{
			const Field& fields_end = map()[map().max_index()];
			for (Field* field = &map()[0]; field < &fields_end; ++field)
				if (BaseImmovable* const imm = field->get_immovable()) {
					if (upcast(Building const, building, imm))
						if (field != &map()[building->get_position()])
							continue;  //  not the building's main position
					imm->load_finish(egbase);
				}
		}
		log("took %ums\n ", timer.ms_since_last_query());

		//  This should be at least after loading Soldiers (Bobs).
		//  NOTE DO NOT CHANGE THE PLACE UNLESS YOU KNOW WHAT ARE YOU DOING
		//  Must be loaded after every kind of object that can see.
		log("Reading Players View Data ... ");
		{
			MapPlayersViewPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		//  This must come before anything that references messages, such as:
		//    * command queue (PlayerMessageCommand, inherited by
		//      Cmd_MessageSetStatusRead and Cmd_MessageSetStatusArchived)
		log("Reading Player Message Data ... ");
		{
			MapPlayersMessagesPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		// Objectives. They are not needed in the Editor, since they are fully
		// defined through Lua scripting. They are also not required for a game,
		// since they will be only be set after it has started.
		log("Reading Objective Data ... ");
		if (!is_game) {
			read_objective_data(*fs_, egbase);
		}
		log("took %ums\n ", timer.ms_since_last_query());
	}

	log("Reading Scripting Data ... ");
	{
		MapScriptingPacket p;
		p.read(*fs_, egbase, is_game, *mol_);
	}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Reading map images ... ");
	load_map_images(*fs_);
	log("took %ums\n ", timer.ms_since_last_query());

	if (load_type != MapLoader::LoadType::kEditor) {
		if (mol_->get_nr_unloaded_objects()) {
			log("WARNING: There are %i unloaded objects. This is a bug, please "
			    "consider committing!\n",
			    mol_->get_nr_unloaded_objects());
		}
	}  // load_type != MapLoader::LoadType::kEditor

	map_.recalc_whole_map(egbase.world());

	map_.ensure_resource_consistency(egbase.world());

	set_state(STATE_LOADED);

	return 0;
}
}
