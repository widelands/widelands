/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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
#include "map_io/map_waterway_packet.h"
#include "map_io/map_waterwaydata_packet.h"
#include "map_io/map_wincondition_packet.h"
#include "map_io/tribes_legacy_lookup_table.h"
#include "map_io/world_legacy_lookup_table.h"
#include "ui_basic/progresswindow.h"

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
	assert(get_state() != State::kLoaded);

	map_.cleanup();

	{
		MapElementalPacket mp;
		mp.pre_read(*fs_, &map_);
		old_world_name_ = mp.old_world_name();
	}
	{
		MapVersionPacket version_packet;
		version_packet.pre_read(*fs_, &map_, false, old_world_name_.empty());
	}

	{
		MapPlayerNamesAndTribesPacket p;
		p.pre_read(*fs_, &map_, !scenario);
	}
	// No scripting/init.lua file -> not playable as scenario
	Map::ScenarioTypes m = Map::NO_SCENARIO;
	if (fs_->file_exists("scripting/init.lua")) {
		m |= Map::SP_SCENARIO;
	}
	if (fs_->file_exists("scripting/multiplayer_init.lua")) {
		m |= Map::MP_SCENARIO;
	}
	map_.set_scenario_types(m);

	set_state(State::kPreLoaded);

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
	Notifications::publish(UI::NoteLoadingMessage(_("Loading map…")));

	const bool is_game = load_type == MapLoader::LoadType::kGame;
	const bool is_editor = load_type == MapLoader::LoadType::kEditor;

	preload_map(!is_game);
	map_.set_size(map_.width_, map_.height_);
	mol_.reset(new MapObjectLoader());

	// MANDATORY PACKETS
	// PRELOAD DATA BEGIN
	auto set_progress_message = [is_editor](std::string text, unsigned step) {
		Notifications::publish(UI::NoteLoadingMessage(
		   (boost::format(_("Loading map: %1$s (%2$u/%3$d)")) % text % step % (is_editor ? 9 : 23))
		      .str()));
	};

	set_progress_message(_("Elemental data"), 1);
	log_info("Reading Elemental Data ... ");
	MapElementalPacket elemental_data_packet;
	elemental_data_packet.read(*fs_, egbase, is_game, *mol_);

	egbase.allocate_player_maps();  //  Can do this now that map size is known.

	//  now player names and tribes
	log_info("Reading Player Names And Tribe Data ... ");
	{
		MapPlayerNamesAndTribesPacket p;
		p.read(*fs_, egbase, is_game, *mol_);
	}
	// PRELOAD DATA END

	if (fs_->file_exists("port_spaces")) {
		log_info("Reading Port Spaces Data ... ");

		MapPortSpacesPacket p;
		p.read(*fs_, egbase, is_game, *mol_);

	}

	log_info("Reading Heights Data ... ");
	set_progress_message(_("Heights"), 2);
	{
		MapHeightsPacket p;
		p.read(*fs_, egbase, is_game, *mol_);
	}

	std::unique_ptr<WorldLegacyLookupTable> world_lookup_table(
	   create_world_legacy_lookup_table(old_world_name_));
	std::unique_ptr<TribesLegacyLookupTable> tribes_lookup_table(new TribesLegacyLookupTable());
	log_info("Reading Terrain Data ... ");
	set_progress_message(_("Terrains"), 3);
	{
		MapTerrainPacket p;
		p.read(*fs_, egbase, *world_lookup_table);
	}

	MapObjectPacket mapobjects;

	log_info("Reading Map Objects ... ");
	set_progress_message(_("Map objects"), 4);
	mapobjects.read(*fs_, egbase, *mol_, *world_lookup_table, *tribes_lookup_table);

	log_info("Reading Player Start Position Data ... ");
	set_progress_message(_("Starting positions"), 5);
	{
		MapPlayerPositionPacket p;
		p.read(*fs_, egbase, is_game, *mol_);
	}

	// This call must stay around forever since this was the way critters have
	// been saved into the map before 2010. Most of the maps we ship are still
	// old in that sense and most maps on the homepage too.
	if (fs_->file_exists("binary/bob")) {
		log_info("Reading (legacy) Bob Data ... ");
		{
			MapBobPacket p;
			p.read(*fs_, egbase, *mol_, *world_lookup_table);
		}
	}

	log_info("Reading Resources Data ... ");
	set_progress_message(_("Resources"), 6);
	{
		MapResourcesPacket p;
		p.read(*fs_, egbase, *world_lookup_table);
	}

	//  NON MANDATORY PACKETS BELOW THIS POINT
	// Do not load unneeded packages in the editor
	if (!is_editor) {
		log_info("Reading Map Version Data ... ");
		set_progress_message(_("Map version"), 7);
		{
			MapVersionPacket p;
			p.read(*fs_, egbase, is_game, old_world_name_.empty());
		}

		set_progress_message(_("Building restrictions"), 8);
		log_info("Reading Allowed Worker Types Data ... ");
		{
			MapAllowedWorkerTypesPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}

		log_info("Reading Allowed Building Types Data ... ");
		{
			MapAllowedBuildingTypesPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}

		set_progress_message(_("Territories"), 9);
		log_info("Reading Node Ownership Data ... ");
		{
			MapNodeOwnershipPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}

		//  !!!!!!!!!! NOTE
		//  This packet must be before any building or road packet. So do not change
		//  this order without knowing what you do
		//  EXISTENT PACKETS
		log_info("Reading Flag Data ... ");
		set_progress_message(_("Flags"), 10);
		{
			MapFlagPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}

		log_info("Reading Road Data ... ");
		set_progress_message(_("Roads and waterways"), 11);
		{
			MapRoadPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}

		log_info("Reading Waterway Data ... ");
		{
			MapWaterwayPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}

		log_info("Reading Building Data ... ");
		set_progress_message(_("Buildings"), 12);
		{
			MapBuildingPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}

		//  DATA PACKETS
		log_info("Reading Flagdata Data ... ");
		set_progress_message(_("Initializing flags"), 13);
		{
			MapFlagdataPacket p;
			p.read(*fs_, egbase, is_game, *mol_, *tribes_lookup_table);
		}

		log_info("Reading Roaddata Data ... ");
		set_progress_message(_("Initializing roads and waterways"), 14);
		{
			MapRoaddataPacket p;
			p.read(*fs_, egbase, is_game, *mol_, *tribes_lookup_table);
		}

		log_info("Reading Waterwaydata Data ... ");
		{
			MapWaterwaydataPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}

		log_info("Reading Buildingdata Data ... ");
		set_progress_message(_("Initializing buildings"), 15);
		{
			MapBuildingdataPacket p;
			p.read(*fs_, egbase, is_game, *mol_, *tribes_lookup_table);
		}

		log_info("Second and third phase loading Map Objects ... ");
		set_progress_message(_("Initializing map objects"), 16);
		mapobjects.load_finish();
		{
			const Field& fields_end = map()[map().max_index()];
			for (Field* field = &map()[0]; field < &fields_end; ++field) {
				if (BaseImmovable* const imm = field->get_immovable()) {
					if (upcast(Building const, building, imm)) {
						if (field != &map()[building->get_position()]) {
							continue;  //  not the building's main position
						}
					}
					imm->load_finish(egbase);
				}
			}
		}

		//  This should be at least after loading Soldiers (Bobs).
		//  NOTE DO NOT CHANGE THE PLACE UNLESS YOU KNOW WHAT ARE YOU DOING
		//  Must be loaded after every kind of object that can see.
		log_info("Reading Players View Data ... ");
		set_progress_message(_("Vision"), 17);
		{
			MapPlayersViewPacket p;
			p.read(*fs_, egbase, *world_lookup_table, *tribes_lookup_table);
		}

		//  This must come before anything that references messages, such as:
		//    * command queue (PlayerMessageCommand, inherited by
		//      Cmd_MessageSetStatusRead and Cmd_MessageSetStatusArchived)
		log_info("Reading Player Message Data ... ");
		set_progress_message(_("Messages"), 18);
		{
			MapPlayersMessagesPacket p;
			p.read(*fs_, egbase, is_game, *mol_);
		}

		// Map data used by win conditions.
		log_info("Reading Wincondition Data ... ");
		set_progress_message(_("Win condition"), 19);
		{
			MapWinconditionPacket p;
			p.read(*fs_, *egbase.mutable_map());
		}

		// Objectives. They are not needed in the Editor, since they are fully
		// defined through Lua scripting. They are also not required for a game,
		// since they will be only be set after it has started.
		log_info("Reading Objective Data ... ");
		set_progress_message(_("Objectives"), 20);
		if (!is_game) {
			read_objective_data(*fs_, egbase);
		}
	}

	log_info("Reading Scripting Data ... ");
	set_progress_message(_("Scripting"), is_editor ? 7 : 21);
	{
		MapScriptingPacket p;
		p.read(*fs_, egbase, is_game, *mol_);
	}

	log_info("Reading map images ... ");
	set_progress_message(_("Images"), is_editor ? 8 : 22);
	load_map_images(*fs_);

	set_progress_message(_("Checking map"), is_editor ? 9 : 23);
	if (!is_editor) {
		if (mol_->get_nr_unloaded_objects()) {
			log_warn("There are %i unloaded objects. This is a bug, please "
			         "consider committing!\n",
			         mol_->get_nr_unloaded_objects());
		}
	}  // load_type != MapLoader::LoadType::kEditor

	map_.recalc_whole_map(egbase);

	map_.ensure_resource_consistency(egbase.world());

	set_state(State::kLoaded);

	return 0;
}
}  // namespace Widelands
