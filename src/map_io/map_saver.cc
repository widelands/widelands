/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "map_io/map_saver.h"

#include <memory>

#include "base/log.h"
#include "base/scoped_timer.h"
#include "base/wexception.h"
#include "graphic/image_io.h"
#include "graphic/minimap_renderer.h"
#include "graphic/texture.h"
#include "io/filesystem/filesystem.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "map_io/map_allowed_building_types_packet.h"
#include "map_io/map_allowed_worker_types_packet.h"
#include "map_io/map_building_packet.h"
#include "map_io/map_buildingdata_packet.h"
#include "map_io/map_elemental_packet.h"
#include "map_io/map_exploration_packet.h"
#include "map_io/map_flag_packet.h"
#include "map_io/map_flagdata_packet.h"
#include "map_io/map_heights_packet.h"
#include "map_io/map_images.h"
#include "map_io/map_node_ownership_packet.h"
#include "map_io/map_object_packet.h"
#include "map_io/map_object_saver.h"
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
#include "ui_basic/progresswindow.h"

namespace Widelands {

MapSaver::MapSaver(FileSystem& fs, EditorGameBase& egbase)
   : egbase_(egbase), fs_(fs), mos_(nullptr) {
}

MapSaver::~MapSaver() {
	delete mos_;
}

void MapSaver::save() {
	const Map& map = egbase_.map();
	std::string timer_message = "MapSaver::save() for '";
	timer_message += map.get_name();
	timer_message += "' took %ums";
	ScopedTimer timer(timer_message);

	bool is_game = is_a(Game, &egbase_);

	auto set_progress_message = [this](std::string s) {
		// Progress messages for the autosave during gameloading
		if (egbase_.get_loader_ui())
			egbase_.get_loader_ui()->step(s);
	};
	set_progress_message(_("Autosaving map…"));

	delete mos_;
	mos_ = new MapObjectSaver();

	// The binary data is saved in an own directory
	// to keep it hidden from the poor debuggers
	fs_.ensure_directory_exists("binary");

	// MANDATORY PACKETS
	// Start with writing the map out, first Elemental data
	// PRELOAD DATA BEGIN
	log("Writing Elemental Data ... ");
	set_progress_message(_("Map autosave: Elemental data (1/23)"));
	{
		MapElementalPacket p;
		p.write(fs_, egbase_, *mos_);
	}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Player Names And Tribe Data ... ");
	{
		MapPlayerNamesAndTribesPacket p;
		p.write(fs_, egbase_, *mos_);
	}
	log("took %ums\n ", timer.ms_since_last_query());
	//  PRELOAD DATA END

	log("Writing Port Spaces Data ... ");
	{
		MapPortSpacesPacket p;
		p.write(fs_, egbase_, *mos_);
	}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Heights Data ... ");
	set_progress_message(_("Map autosave: Heights (2/23)"));
	{
		MapHeightsPacket p;
		p.write(fs_, egbase_, *mos_);
	}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Terrain Data ... ");
	set_progress_message(_("Map autosave: Terrains (3/23)"));
	{
		MapTerrainPacket p;
		p.write(fs_, egbase_);
	}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Player Start Position Data ... ");
	set_progress_message(_("Map autosave: Starting positions (4/23)"));
	{
		MapPlayerPositionPacket p;
		p.write(fs_, egbase_, *mos_);
	}
	log("took %ums\n ", timer.ms_since_last_query());

	// We don't save these when saving a map in the editor.
	if (is_game) {
		//  This must come before anything that references messages, such as:
		//    * command queue (PlayerMessageCommand, inherited by
		//      Cmd_MessageSetStatusRead and Cmd_MessageSetStatusArchived)
		log("Writing Player Message Data ... ");
		set_progress_message(_("Map autosave: Messages (5/23)"));
		{
			MapPlayersMessagesPacket p;
			p.write(fs_, egbase_, *mos_);
		}
		log("took %ums\n ", timer.ms_since_last_query());
	}

	log("Writing Resources Data ... ");
	set_progress_message(_("Map autosave: Resources (6/23)"));
	{
		MapResourcesPacket p;
		p.write(fs_, egbase_);
	}
	log("took %ums\n ", timer.ms_since_last_query());

	//  NON MANDATORY PACKETS BELOW THIS POINT
	log("Writing Map Version ... ");
	set_progress_message(_("Map autosave: Map version (7/23)"));
	{
		MapVersionPacket p;
		p.write(fs_, egbase_);
	}
	log("took %ums\n ", timer.ms_since_last_query());

	// We don't save these when saving a map in the editor.
	if (is_game) {
		PlayerNumber const nr_players = map.get_nrplayers();

		//  allowed worker types
		log("Writing Allowed Worker Types Data ... ");
		set_progress_message(_("Map autosave: Building restrictions (8/23)"));
		{
			MapAllowedWorkerTypesPacket p;
			p.write(fs_, egbase_, *mos_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		//  allowed building types
		iterate_players_existing_const(plnum, nr_players, egbase_, player) {
			for (DescriptionIndex i = 0; i < egbase_.tribes().nrbuildings(); ++i) {
				if (!player->is_building_type_allowed(i)) {
					log("Writing Allowed Building Types Data ... ");
					MapAllowedBuildingTypesPacket p;
					p.write(fs_, egbase_, *mos_);
					log("took %ums\n ", timer.ms_since_last_query());
					goto end_find_a_forbidden_building_type_loop;
				}
			}
		}
	end_find_a_forbidden_building_type_loop:;

		// !!!!!!!!!! NOTE
		// This packet must be written before any building or road packet. So do not
		// change this order unless you know what you are doing
		// EXISTING PACKETS
		log("Writing Flag Data ... ");
		set_progress_message(_("Map autosave: Flags (9/23)"));
		{
			MapFlagPacket p;
			p.write(fs_, egbase_, *mos_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		log("Writing Road Data ... ");
		set_progress_message(_("Map autosave: Roads and waterways (10/23)"));
		{
			MapRoadPacket p;
			p.write(fs_, egbase_, *mos_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		log("Writing Waterway Data ... ");
		{
			MapWaterwayPacket p;
			p.write(fs_, egbase_, *mos_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		log("Writing Building Data ... ");
		set_progress_message(_("Map autosave: Buildings (11/23)"));
		{
			MapBuildingPacket p;
			p.write(fs_, egbase_, *mos_);
		}
		log("took %ums\n ", timer.ms_since_last_query());
	}

	// We do need to save this one in the editor!
	log("Writing Map Objects ... ");
	set_progress_message(_("Map autosave: Map objects (12/23)"));
	{
		MapObjectPacket p;
		p.write(fs_, egbase_, *mos_);
	}
	log("took %ums\n ", timer.ms_since_last_query());

	if (is_game) {
		// Map data used by win conditions
		log("Writing Wincondition Data ... ");
		set_progress_message(_("Map autosave: Win conditions (13/23)"));
		{
			MapWinconditionPacket p;
			p.write(fs_, *egbase_.mutable_map(), *mos_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		// DATA PACKETS
		if (mos_->get_nr_flags()) {
			log("Writing Flagdata Data ... ");
			set_progress_message(_("Map autosave: Flag details (14/23)"));
			{
				MapFlagdataPacket p;
				p.write(fs_, egbase_, *mos_);
			}
			log("took %ums\n ", timer.ms_since_last_query());
		}

		if (mos_->get_nr_roads()) {
			log("Writing Roaddata Data ... ");
			set_progress_message(_("Map autosave: Road and waterway details (15/23)"));
			{
				MapRoaddataPacket p;
				p.write(fs_, egbase_, *mos_);
			}
			log("took %ums\n ", timer.ms_since_last_query());
		}

		if (mos_->get_nr_waterways()) {
			log("Writing Waterwaydata Data ... ");
			{
				MapWaterwaydataPacket p;
				p.write(fs_, egbase_, *mos_);
			}
			log("took %ums\n ", timer.ms_since_last_query());
		}

		if (mos_->get_nr_buildings()) {
			log("Writing Buildingdata Data ... ");
			set_progress_message(_("Map autosave: Building details (16/23)"));
			{
				MapBuildingdataPacket p;
				p.write(fs_, egbase_, *mos_);
			}
			log("took %ums\n ", timer.ms_since_last_query());
		}

		log("Writing Node Ownership Data ... ");
		set_progress_message(_("Map autosave: Territory (17/23)"));
		{
			MapNodeOwnershipPacket p;
			p.write(fs_, egbase_, *mos_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		log("Writing Exploration Data ... ");
		set_progress_message(_("Map autosave: Exploration (18/23)"));
		{
			MapExplorationPacket p;
			p.write(fs_, egbase_, *mos_);
		}
		log("took %ums\n ", timer.ms_since_last_query());

		log("Writing Players Unseen Data ... ");
		set_progress_message(_("Map autosave: Vision (19/23)"));
		{
			MapPlayersViewPacket p;
			p.write(fs_, egbase_, *mos_);
		}
		log("took %ums\n ", timer.ms_since_last_query());
	}

	// We also want to write these in the editor.
	log("Writing Scripting Data ... ");
	set_progress_message(_("Map autosave: Scripting (20/23)"));
	{
		MapScriptingPacket p;
		p.write(fs_, egbase_, *mos_);
	}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Objective Data ... ");
	set_progress_message(_("Map autosave: Objectives (21/23)"));
	write_objective_data(fs_, egbase_);
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing map images ... ");
	set_progress_message(_("Map autosave: Images (22/23)"));
	save_map_images(&fs_, map.filesystem());
	log("took %ums\n ", timer.ms_since_last_query());

#ifndef NDEBUG
	if (is_game) {
		mos_->detect_unsaved_objects();
	}
#endif

	// Write minimap
	{
		set_progress_message(_("Map autosave: Minimap (23/23)"));
		std::unique_ptr<Texture> minimap(
		   draw_minimap(egbase_, nullptr, Rectf(), MiniMapType::kStaticMap, MiniMapLayer::Terrain));
		FileWrite fw;
		save_to_png(minimap.get(), &fw, ColorType::RGBA);
		fw.write(fs_, "minimap.png");
	}
}

}  // namespace Widelands
