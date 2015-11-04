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
#include "logic/player.h"
#include "logic/tribes/tribe_descr.h"
#include "map_io/map_allowed_building_types_packet.h"
#include "map_io/map_allowed_worker_types_packet.h"
#include "map_io/map_building_packet.h"
#include "map_io/map_buildingdata_packet.h"
#include "map_io/map_elemental_packet.h"
#include "map_io/map_exploration_packet.h"
#include "map_io/map_extradata_packet.h"
#include "map_io/map_flag_packet.h"
#include "map_io/map_flagdata_packet.h"
#include "map_io/map_heights_packet.h"
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


namespace Widelands {

MapSaver::MapSaver(FileSystem & fs, EditorGameBase & egbase)
	: m_egbase(egbase), m_fs(fs), m_mos(nullptr)
{}


MapSaver::~MapSaver() {delete m_mos;}


void MapSaver::save() {
	ScopedTimer timer("MapSaver::save() took %ums");

	delete m_mos;
	m_mos = new MapObjectSaver();

	// The binary data is saved in an own directory
	// to keep it hidden from the poor debuggers
	m_fs.ensure_directory_exists("binary");

	// MANDATORY PACKETS
	// Start with writing the map out, first Elemental data
	// PRELOAD DATA BEGIN
	log("Writing Elemental Data ... ");
	{MapElementalPacket               p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Player Names And Tribe Data ... ");
	{MapPlayerNamesAndTribesPacket p; p.write(m_fs, m_egbase, *m_mos);
	}
	log("took %ums\n ", timer.ms_since_last_query());
	//  PRELOAD DATA END

	log("Writing Port Spaces Data ... ");
	{MapPortSpacesPacket             p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Heights Data ... ");
	{MapHeightsPacket                 p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Terrain Data ... ");
	{MapTerrainPacket                 p; p.write(m_fs, m_egbase);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Player Start Position Data ... ");
	{MapPlayerPositionPacket         p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	//  This must come before anything that references messages, such as:
	//    * command queue (PlayerMessageCommand, inherited by
	//      Cmd_MessageSetStatusRead and Cmd_MessageSetStatusArchived)
	log("Writing Player Message Data ... ");
	{MapPlayersMessagesPacket        p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Resources Data ... ");
	{MapResourcesPacket               p; p.write(m_fs, m_egbase);}
	log("took %ums\n ", timer.ms_since_last_query());

	//  NON MANDATORY PACKETS BELOW THIS POINT
	log("Writing Map Extra Data ... ");
	{MapExtradataPacket               p; p.write(m_fs, m_egbase);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Map Version ... ");
	{MapVersionPacket               p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());


	const Map & map = m_egbase.map();

	PlayerNumber const nr_players = map.get_nrplayers();

	//  allowed worker types
	log("Writing Allowed Worker Types Data ... ");
	{MapAllowedWorkerTypesPacket p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

 //  allowed building types
	iterate_players_existing_const(plnum, nr_players, m_egbase, player) {
		for (BuildingIndex i = 0; i < m_egbase.tribes().nrbuildings(); ++i) {
			if (!player->is_building_type_allowed(i)) {
				log("Writing Allowed Building Types Data ... ");
				MapAllowedBuildingTypesPacket p;
				p                                  .write(m_fs, m_egbase, *m_mos);
				log("took %ums\n ", timer.ms_since_last_query());
				goto end_find_a_forbidden_building_type_loop;
			}
		}
	} end_find_a_forbidden_building_type_loop:;

	// !!!!!!!!!! NOTE
	// This packet must be before any building or road packet. So do not
	// change this order without knowing what you do
	// EXISTENT PACKETS
	log("Writing Flag Data ... ");
	{MapFlagPacket                   p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Road Data ... ");
	{MapRoadPacket                   p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Building Data ... ");
	{MapBuildingPacket               p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Map Objects ... ");
	{MapObjectPacket                      p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	// DATA PACKETS
	if (m_mos->get_nr_flags()) {
		log("Writing Flagdata Data ... ");
		{MapFlagdataPacket            p; p.write(m_fs, m_egbase, *m_mos);}
		log("took %ums\n ", timer.ms_since_last_query());
	}

	if (m_mos->get_nr_roads()) {
		log("Writing Roaddata Data ... ");
		{MapRoaddataPacket            p; p.write(m_fs, m_egbase, *m_mos);}
		log("took %ums\n ", timer.ms_since_last_query());
	}

	if (m_mos->get_nr_buildings()) {
		log("Writing Buildingdata Data ... ");
		{MapBuildingdataPacket        p; p.write(m_fs, m_egbase, *m_mos);}
		log("took %ums\n ", timer.ms_since_last_query());
	}

	log("Writing Node Ownership Data ... ");
	{MapNodeOwnershipPacket         p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Exploration Data ... ");
	{MapExplorationPacket            p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Players Unseen Data ... ");
	{MapPlayersViewPacket           p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Scripting Data ... ");
	{MapScriptingPacket              p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

	log("Writing Objective Data ... ");
	{MapObjectivePacket              p; p.write(m_fs, m_egbase, *m_mos);}
	log("took %ums\n ", timer.ms_since_last_query());

#ifndef NDEBUG
	m_mos->detect_unsaved_objects();
#endif

	// Write minimap
	{
		std::unique_ptr<Texture> minimap(
		   draw_minimap(m_egbase, nullptr, Point(0, 0), MiniMapLayer::Terrain));
		FileWrite fw;
		save_to_png(minimap.get(), &fw, ColorType::RGBA);
		fw.write(m_fs, "minimap.png");
	}
}

}
