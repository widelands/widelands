/*
 * Copyright (C) 2007 by the Widelands Development Team
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

#include "editor_game_base.h"
#include "fileread.h"
#include "filewrite.h"
#include "map.h"
#include "player.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include "filesystem.h"
#include "zip_exceptions.h"


#define CURRENT_PACKET_VERSION   1
#define PLAYERDIRNAME_TEMPLATE "player/%u"
#define DIRNAME_TEMPLATE PLAYERDIRNAME_TEMPLATE "/binary"
#define FILENAME_TEMPLATE DIRNAME_TEMPLATE "/areawatchers"
#define FILENAME_SIZE 32


void Widelands_Map_Players_AreaWatchers_Data_Packet::Read
(FileSystem       & fs,
 Editor_Game_Base * egbase,
 const bool         skip,
 Widelands_Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip) return;

	const Map & map = egbase->map();
	const Extent extent = egbase->map().extent();
	const Player_Number nr_players = map.get_nrplayers();
	iterate_players_existing(p, nr_players, *egbase, player) {
		char filename[FILENAME_SIZE];
		snprintf(filename, sizeof(filename), FILENAME_TEMPLATE, p);
		FileRead fr;
		struct Not_Found {};
		try {
			try {fr.Open(fs, filename);}
			catch (const File_error         &) {throw Not_Found();}
			catch (const ZipOperation_error &) {throw Not_Found();}
		} catch (const Not_Found) {continue;}
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			while (not fr.IsEOF()) {
				const uint32_t reg = fr.Unsigned32();
				if (ol->is_object_known(reg))
					throw wexception
						("Widelands_Map_Players_AreaWatchers_Data_Packet::Read: "
						 "player %u: in \"%s\":%u: read object with reg %u, but an "
						 "object with that reg has already been loaded",
						 p, filename, fr.GetPrevPos(), reg);
				Coords c;
				try {c = fr.Coords32(extent);}
				catch  (const FileRead::Width_Exceeded e) {
					throw wexception
						("Widelands_Map_Players_AreaWatchers_Data_Packet::Read: "
						 "player %u: in \"%s\":%u: area watcher %u: x-cordinate (%u) "
						 "exceeds the witdh of the map (%u)\n",
						 p, filename, e.position, reg, e.x, e.w);
				} catch (const FileRead::Height_Exceeded e) {
					throw wexception
						("Widelands_Map_Players_AreaWatchers_Data_Packet::Read: "
						 "player %u: in \"%s\":%u: area watcher %u: y-cordinate (%u) "
						 "exceeds the height of the map (%u)\n",
						 p, filename, e.position, reg, e.y, e.h);
				}
				ol->register_object
					(egbase,
					 reg,
					 &player->add_areawatcher
					 (Player_Area<>(p, Area<>(c, fr.Unsigned16()))));
			}
		} else
			throw wexception
				("Widelands_Map_Players_AreaWatchers_Data_Packet::Read: player %u: "
				 "in \"%s\":0: unknown packet version %u",
				 p, filename, packet_version);
	}
}


void Widelands_Map_Players_AreaWatchers_Data_Packet::Write
(FileSystem                     &       fs,
 Editor_Game_Base               *       egbase,
 Widelands_Map_Map_Object_Saver * const os)
throw (_wexception)
{
	fs.EnsureDirectoryExists("player");
	const Map & map = egbase->map();
	const Player_Number nr_players = map.get_nrplayers();
	iterate_players_existing_const(p, nr_players, *egbase, player) {
		FileWrite fw;
		fw.Unsigned16(CURRENT_PACKET_VERSION);
		const Player::AreaWatchers & areawatchers = player->areawatchers();
		const Player::AreaWatchers::const_iterator areawatchers_end =
			areawatchers.end();
		for
			(Player::AreaWatchers::const_iterator it = areawatchers.begin();
			 it != areawatchers_end;
			 ++it)
		{
			const AreaWatcher & areawatcher =
				dynamic_cast<const AreaWatcher &>(*it->get(egbase));
			fw.Unsigned32(os->register_object(&areawatcher));
			fw.Coords32  (areawatcher);
			fw.Unsigned16(areawatcher.radius);
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
