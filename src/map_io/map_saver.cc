/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "map_io/map_saver.h"

#include <memory>

#include "base/log.h"
#include "base/multithreading.h"
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
	ScopedTimer timer(timer_message, true);

	const bool is_game = egbase_.is_game();

	auto set_progress_message = [](std::string text, int step) {
		Notifications::publish(UI::NoteLoadingMessage(
		   step < 0 ? text : format(_("Saving map: %1$s (%2$d/%3$d)"), text, step, 22)));
	};
	set_progress_message(_("Autosaving map…"), -1);

	delete mos_;
	mos_ = new MapObjectSaver();

	// The binary data is saved in an own directory
	// to keep it hidden from the poor debuggers
	fs_.ensure_directory_exists("binary");

	// MANDATORY PACKETS
	// Start with writing the map out, first Elemental data
	// PRELOAD DATA BEGIN
	verb_log_info_time(egbase_.get_gametime(), "Writing Elemental Data ... ");
	set_progress_message(_("Elemental data"), 1);
	{
		MapElementalPacket p;
		p.write(fs_, egbase_, *mos_);
	}

	verb_log_info_time(egbase_.get_gametime(), "Writing Player Names And Tribe Data ... ");
	{
		MapPlayerNamesAndTribesPacket p;
		p.write(fs_, egbase_, *mos_);
	}
	//  PRELOAD DATA END

	verb_log_info_time(egbase_.get_gametime(), "Writing Port Spaces Data ... ");
	{
		MapPortSpacesPacket p;
		p.write(fs_, egbase_, *mos_);
	}

	verb_log_info_time(egbase_.get_gametime(), "Writing Heights Data ... ");
	set_progress_message(_("Heights"), 2);
	{
		MapHeightsPacket p;
		p.write(fs_, egbase_, *mos_);
	}

	verb_log_info_time(egbase_.get_gametime(), "Writing Terrain Data ... ");
	set_progress_message(_("Terrains"), 3);
	{
		MapTerrainPacket p;
		p.write(fs_, egbase_);
	}

	verb_log_info_time(egbase_.get_gametime(), "Writing Player Start Position Data ... ");
	set_progress_message(_("Starting positions"), 4);
	{
		MapPlayerPositionPacket p;
		p.write(fs_, egbase_, *mos_);
	}

	// We don't save these when saving a map in the editor.
	if (is_game) {
		//  This must come before anything that references messages, such as:
		//    * command queue (PlayerMessageCommand, inherited by
		//      Cmd_MessageSetStatusRead and Cmd_MessageSetStatusArchived)
		verb_log_info_time(egbase_.get_gametime(), "Writing Player Message Data ... ");
		set_progress_message(_("Messages"), 5);
		{
			MapPlayersMessagesPacket p;
			p.write(fs_, egbase_, *mos_);
		}
	}

	verb_log_info_time(egbase_.get_gametime(), "Writing Resources Data ... ");
	set_progress_message(_("Resources"), 6);
	{
		MapResourcesPacket p;
		p.write(fs_, egbase_);
	}

	//  NON MANDATORY PACKETS BELOW THIS POINT
	verb_log_info_time(egbase_.get_gametime(), "Writing Map Version ... ");
	set_progress_message(_("Map version"), 7);
	{
		MapVersionPacket p;
		p.write(fs_, egbase_);
	}

	// We don't save these when saving a map in the editor.
	if (is_game) {
		PlayerNumber const nr_players = map.get_nrplayers();

		//  allowed worker types
		verb_log_info_time(egbase_.get_gametime(), "Writing Allowed Worker Types Data ... ");
		set_progress_message(_("Building restrictions"), 8);
		{
			MapAllowedWorkerTypesPacket p;
			p.write(fs_, egbase_, *mos_);
		}

		//  allowed building types
		iterate_players_existing_const(plnum, nr_players, egbase_, player) {
			for (DescriptionIndex i = 0; i < egbase_.descriptions().nr_buildings(); ++i) {
				if (!player->is_building_type_allowed(i)) {
					verb_log_info_time(
					   egbase_.get_gametime(), "Writing Allowed Building Types Data ... ");
					MapAllowedBuildingTypesPacket p;
					p.write(fs_, egbase_, *mos_);
					goto end_find_a_forbidden_building_type_loop;
				}
			}
		}
	end_find_a_forbidden_building_type_loop:;

		// !!!!!!!!!! NOTE
		// This packet must be written before any building or road packet. So do not
		// change this order unless you know what you are doing
		// EXISTING PACKETS
		verb_log_info_time(egbase_.get_gametime(), "Writing Flag Data ... ");
		set_progress_message(_("Flags"), 9);
		{
			MapFlagPacket p;
			p.write(fs_, egbase_, *mos_);
		}

		verb_log_info_time(egbase_.get_gametime(), "Writing Road Data ... ");
		set_progress_message(_("Roads and waterways"), 10);
		{
			MapRoadPacket p;
			p.write(fs_, egbase_, *mos_);
		}

		verb_log_info_time(egbase_.get_gametime(), "Writing Waterway Data ... ");
		{
			MapWaterwayPacket p;
			p.write(fs_, egbase_, *mos_);
		}

		verb_log_info_time(egbase_.get_gametime(), "Writing Building Data ... ");
		set_progress_message(_("Buildings"), 11);
		{
			MapBuildingPacket p;
			p.write(fs_, egbase_, *mos_);
		}
	}

	// We do need to save this one in the editor!
	verb_log_info_time(egbase_.get_gametime(), "Writing Map Objects ... ");
	set_progress_message(_("Map objects"), 12);
	{
		MapObjectPacket p;
		p.write(fs_, egbase_, *mos_);
	}

	if (is_game) {
		// Map data used by win conditions
		verb_log_info_time(egbase_.get_gametime(), "Writing Wincondition Data ... ");
		set_progress_message(_("Win condition"), 13);
		{
			MapWinconditionPacket p;
			p.write(fs_, *egbase_.mutable_map());
		}

		// DATA PACKETS
		if (mos_->get_nr_flags() != 0u) {
			verb_log_info_time(egbase_.get_gametime(), "Writing Flagdata Data ... ");
			set_progress_message(_("Flag details"), 14);
			{
				MapFlagdataPacket p;
				p.write(fs_, egbase_, *mos_);
			}
		}

		if (mos_->get_nr_roads() != 0u) {
			verb_log_info_time(egbase_.get_gametime(), "Writing Roaddata Data ... ");
			set_progress_message(_("Road and waterway details"), 15);
			{
				MapRoaddataPacket p;
				p.write(fs_, egbase_, *mos_);
			}
		}

		if (mos_->get_nr_waterways() != 0u) {
			verb_log_info_time(egbase_.get_gametime(), "Writing Waterwaydata Data ... ");
			{
				MapWaterwaydataPacket p;
				p.write(fs_, egbase_, *mos_);
			}
		}

		if (mos_->get_nr_buildings() != 0u) {
			verb_log_info_time(egbase_.get_gametime(), "Writing Buildingdata Data ... ");
			set_progress_message(_("Building details"), 16);
			{
				MapBuildingdataPacket p;
				p.write(fs_, egbase_, *mos_);
			}
		}

		verb_log_info_time(egbase_.get_gametime(), "Writing Node Ownership Data ... ");
		set_progress_message(_("Territory"), 17);
		{
			MapNodeOwnershipPacket p;
			p.write(fs_, egbase_, *mos_);
		}

		verb_log_info_time(egbase_.get_gametime(), "Writing Players View Data ... ");
		set_progress_message(_("Vision"), 18);
		{
			MapPlayersViewPacket p;
			p.write(fs_, egbase_);
		}
	}

	// We also want to write these in the editor.
	verb_log_info_time(egbase_.get_gametime(), "Writing Scripting Data ... ");
	set_progress_message(_("Scripting"), 19);
	{
		MapScriptingPacket p;
		p.write(fs_, egbase_, *mos_);
	}

	verb_log_info_time(egbase_.get_gametime(), "Writing Objective Data ... ");
	set_progress_message(_("Objectives"), 20);
	write_objective_data(fs_, egbase_);

	verb_log_info_time(egbase_.get_gametime(), "Writing map images ... ");
	set_progress_message(_("Images"), 21);
	save_map_images(&fs_, map.filesystem());

#ifndef NDEBUG
	if (is_game) {
		mos_->detect_unsaved_objects();
	}
#endif

	// Write minimap
	set_progress_message(_("Minimap"), 22);
	NoteThreadSafeFunction::instantiate(
	   [this]() {
		   std::unique_ptr<Texture> minimap(draw_minimap(
		      egbase_, nullptr, Rectf(), MiniMapType::kStaticMap, MiniMapLayer::Terrain));
		   FileWrite fw;
		   save_to_png(minimap.get(), &fw, ColorType::RGBA);
		   fw.write(fs_, "minimap.png");
	   },
	   true);
}

}  // namespace Widelands
