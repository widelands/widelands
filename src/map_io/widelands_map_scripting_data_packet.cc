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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/world.h"
#include "profile/profile.h"
#include "scripting/scripting.h"
#include "upcast.h"

#include "widelands_map_scripting_data_packet.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1

/*
 * ========================================================================
 *            PUBLIC IMPLEMENTATION
 * ========================================================================
 */
void Map_Scripting_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool is_normal_game,
	 Map_Map_Object_Loader &       mol)
throw (_wexception)
{
	if (not is_normal_game) { // Only load scripting stuff if this is a scenario
		egbase.lua().register_scripts(fs, "map");
	}

	// Always try to load the global State: even in a normal game, some lua
	// coroutines could run. But make sure that this is really a game, other
	// wise this makes no sense.
	upcast(Game, g, &egbase);
	Widelands::FileRead fr;
	if (g and fr.TryOpen(fs, "scripting/globals.dump")) {
		upcast(LuaGameInterface, lgi, &g->lua());
		lgi->read_global_env(fr, mol, fr.Unsigned32());
	}
}


void Map_Scripting_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
throw (_wexception)
{
	ScriptContainer & p = egbase.lua().get_scripts_for("map");

	fs.EnsureDirectoryExists("scripting");

	for (ScriptContainer::iterator i = p.begin(); i != p.end(); i++) {
		std::string fname = "scripting/";
		fname += i->first;
		fname += ".lua";

		fs.Write(fname, i->second.c_str(), i->second.size());
	}

	// Dump the global environment if this is a game and not in the editor
	if (upcast(Game, g, &egbase)) {

		Widelands::FileWrite fw;
		Widelands::FileWrite::Pos pos = fw.GetPos();
		fw.Unsigned32(0); // N bytes written, follows below

		upcast(LuaGameInterface, lgi, &g->lua());
		uint32_t nwritten = Little32(lgi->write_global_env(fw, mos));
		fw.Data(&nwritten, 4, pos);

		fw.Write(fs, "scripting/globals.dump");
	}
}

}
