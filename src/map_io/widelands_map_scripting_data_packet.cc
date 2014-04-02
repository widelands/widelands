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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "map_io/widelands_map_scripting_data_packet.h"

#include <string>

#include <boost/algorithm/string/predicate.hpp>

#include "helper.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "profile/profile.h"
#include "scripting/scripting.h"
#include "upcast.h"

namespace Widelands {

namespace {
const int SCRIPTING_DATA_PACKET_VERSION = 1;
}  // namespace
/*
 * ========================================================================
 *            PUBLIC IMPLEMENTATION
 * ========================================================================
 */
void Map_Scripting_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool,
	 Map_Map_Object_Loader &       mol)
{
	// Always try to load the global State: even in a normal game, some lua
	// coroutines could run. But make sure that this is really a game, other
	// wise this makes no sense.
	upcast(Game, g, &egbase);
	Widelands::FileRead fr;
	if (g and fr.TryOpen(fs, "scripting/globals.dump"))
	{
		const uint32_t sentinel = fr.Unsigned32();
		const uint32_t packet_version = fr.Unsigned32();
		if (sentinel != 0xDEADBEEF && packet_version != SCRIPTING_DATA_PACKET_VERSION) {
			throw game_data_error(
			   "This savegame is from an older version of Widelands and can not be loaded any more.");
		}
		upcast(LuaGameInterface, lgi, &g->lua());
		lgi->read_global_env(fr, mol, fr.Unsigned32());
	}
}


void Map_Scripting_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fs.EnsureDirectoryExists("scripting");

	FileSystem* map_fs = egbase.map().filesystem();
	if (map_fs) {
		for (const std::string& script :
		     filter(map_fs->ListDirectory("scripting"),
		            [](const std::string& fn) {
							return boost::ends_with(fn, ".lua");
						})) {
			size_t length;
			void* input_data = map_fs->Load(script, length);
			fs.Write(script, input_data, length);
			free(input_data);
		}
	}

	// Dump the global environment if this is a game and not in the editor
	if (upcast(Game, g, &egbase)) {
		Widelands::FileWrite fw;
		fw.Unsigned32(0xDEADBEEF);  // Sentinel, because there was no packet version.
		fw.Unsigned32(SCRIPTING_DATA_PACKET_VERSION);
		const Widelands::FileWrite::Pos pos = fw.GetPos();
		fw.Unsigned32(0); // N bytes written, follows below

		upcast(LuaGameInterface, lgi, &g->lua());
		uint32_t nwritten = Little32(lgi->write_global_env(fw, mos));
		fw.Data(&nwritten, 4, pos);

		fw.Write(fs, "scripting/globals.dump");
	}
}

}
