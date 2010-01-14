/*
 * Copyright (C) 2007-2008, 2010 by the Widelands Development Team
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

#include "widelands_map_players_areawatchers_data_packet.h"

#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION   1
#define PLAYERDIRNAME_TEMPLATE "player/%u"
#define DIRNAME_TEMPLATE PLAYERDIRNAME_TEMPLATE "/binary"
#define FILENAME_TEMPLATE DIRNAME_TEMPLATE "/areawatchers"
#define FILENAME_SIZE 32

void Map_Players_AreaWatchers_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader &       mol)
throw (_wexception)
{
	if (skip)
		return;

	Map const & map = egbase.map();
	Extent const extent = map.extent();
	const Player_Number nr_players = map.get_nrplayers();
	iterate_players_existing(p, nr_players, egbase, player) {
		char filename[FILENAME_SIZE];
		snprintf(filename, sizeof(filename), FILENAME_TEMPLATE, p);
		FileRead fr;
		struct Not_Found {};

		if (!fr.TryOpen(fs, filename))
			continue;

		try {
			uint16_t const packet_version = fr.Unsigned16();
			if (packet_version == CURRENT_PACKET_VERSION) {
				while (not fr.EndOfFile()) {
					uint32_t const reg = fr.Unsigned32();
					if (mol.is_object_known(reg))
						throw game_data_error
							("%lu: read object with reg %u, but an object with that "
							 "reg has already been loaded",
							 static_cast<long unsigned int>(fr.GetPos() - 4), reg);
					Coords c;
					try {c = fr.Coords32(extent);}
					catch (game_data_error const & e) {
						throw game_data_error
							("%lu: coordinates of watcher %u: %s",
							 static_cast<long unsigned int>(fr.GetPos() - 4), reg,
							 e.what());
					}
					mol.register_object
						(reg,
						 player->add_areawatcher
						 	(Player_Area<>(p, Area<>(c, fr.Unsigned16()))));
				}
			} else
				throw game_data_error
					("0: unknown/unhandled packet version %u", packet_version);
		} catch (std::exception const & e) {
			throw game_data_error
				("Map_Players_AreaWatchers_Data_Packet::Read: player %u: in "
				 "\"%s\": %s",
				 p, filename, e.what());
		}
	}
}


void Map_Players_AreaWatchers_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
throw (_wexception)
{
	fs.EnsureDirectoryExists("player");
	Map const & map = egbase.map();
	const Player_Number nr_players = map.get_nrplayers();
	iterate_players_existing_const(p, nr_players, egbase, player) {
		FileWrite fw;
		fw.Unsigned16(CURRENT_PACKET_VERSION);
		const Player::AreaWatchers & areawatchers = player->areawatchers();
		container_iterate_const(Player::AreaWatchers, areawatchers, i) {
			AreaWatcher const & areawatcher = *i.current->get(egbase);
			fw.Unsigned32(mos.register_object(areawatcher));
			fw.Area48    (areawatcher);
			mos.mark_object_as_saved(areawatcher);
		}
		char filename[FILENAME_SIZE];
		snprintf(filename, sizeof(filename), PLAYERDIRNAME_TEMPLATE, p);
		fs.EnsureDirectoryExists(filename);
		snprintf(filename, sizeof(filename),       DIRNAME_TEMPLATE, p);
		fs.EnsureDirectoryExists(filename);
		snprintf(filename, sizeof(filename),      FILENAME_TEMPLATE, p);
		fw.Write(fs, filename);
	}
}

}
