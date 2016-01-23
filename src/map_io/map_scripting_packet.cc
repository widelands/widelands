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

#include "map_io/map_scripting_packet.h"

#include <string>

#include <boost/algorithm/string/predicate.hpp>

#include "base/macros.h"
#include "helper.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/map_objects/world/world.h"
#include "scripting/logic.h"

namespace Widelands {

namespace {
constexpr uint32_t kCurrentPacketVersion = 2;
}  // namespace
/*
 * ========================================================================
 *            PUBLIC IMPLEMENTATION
 * ========================================================================
 */
void MapScriptingPacket::read
	(FileSystem& fs,
	 EditorGameBase& egbase,
	 bool,
	 MapObjectLoader& mol)
{
	// Always try to load the global State: even in a normal game, some lua
	// coroutines could run. But make sure that this is really a game, other
	// wise this makes no sense.
	upcast(Game, g, &egbase);
	FileRead fr;
	if (g && fr.try_open(fs, "scripting/globals.dump"))
	{
		try {
			const uint32_t packet_version = fr.unsigned_32();
			if (packet_version == kCurrentPacketVersion) {
				upcast(LuaGameInterface, lgi, &g->lua());
				lgi->read_global_env(fr, mol, fr.unsigned_32());
			} else {
				throw UnhandledVersionError("MapScriptingPacket", packet_version, kCurrentPacketVersion);
			}
		} catch (const WException & e) {
			throw GameDataError("scripting: %s", e.what());
		}
	}
}


void MapScriptingPacket::write
	(FileSystem & fs, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fs.ensure_directory_exists("scripting");

	FileSystem* map_fs = egbase.map().filesystem();
	if (map_fs) {
		for (const std::string& script :
			  filter(map_fs->list_directory("scripting"),
		            [](const std::string& fn) {
							return boost::ends_with(fn, ".lua");
						})) {
			size_t length;
			void* input_data = map_fs->load(script, length);
			fs.write(script, input_data, length);
			free(input_data);
		}
	}

	// Dump the global environment if this is a game and not in the editor
	if (upcast(Game, g, &egbase)) {
		FileWrite fw;
		fw.unsigned_32(kCurrentPacketVersion);
		const FileWrite::Pos pos = fw.get_pos();
		fw.unsigned_32(0); // N bytes written, follows below

		upcast(LuaGameInterface, lgi, &g->lua());
		uint32_t nwritten = little_32(lgi->write_global_env(fw, mos));
		fw.data(&nwritten, 4, pos);

		fw.write(fs, "scripting/globals.dump");
	}
}

}
