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


#include <algorithm>
#include <cctype>  // std::tolower

#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/world.h"
#include "profile/profile.h"
#include "scripting/scripting.h"

#include "widelands_map_scripting_data_packet.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1

/*
 * ========================================================================
 *            PRIVATE FUNCTIONS
 * ========================================================================
 */
static bool m_filename_to_short(const std::string & s) {
	return s.size() < 4;
}
static bool m_is_lua_file(const std::string & s) {
	std::string ext = s.substr(s.size() - 4, s.size());
	// explicit cast needed to resolve ambiguity
	std::transform
		(ext.begin(), ext.end(), ext.begin(),
		 static_cast<int(*)(int)>(std::tolower)
	);
	return (ext == ".lua");
}


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
throw (_wexception)
{
	filenameset_t scripting_files;

	// Theoretically, we should be able to use fs.FindFiles(*.lua) here,
	// but since FindFiles doesn't support Globbing in Zips and most
	// saved maps/games are zip, we have to work around this issue.
	fs.FindFiles("scripting", "*", &scripting_files);

	for
		(filenameset_t::iterator i = scripting_files.begin();
		 i != scripting_files.end(); i++)
	{
		if (m_filename_to_short(*i) or not m_is_lua_file(*i))
			continue;

		size_t length;
		std::string data(static_cast<char *>(fs.Load(*i, length)));
		std::string name = i->substr(0, i->size() - 4); // strips '.lua'
		size_t pos = name.rfind('/');
		if (pos == std::string::npos)
			pos = name.rfind("\\");
		if (pos != std::string::npos)
			name = name.substr(pos + 1, name.size());

		log("Registering script: (map,%s)\n", name.c_str());

		egbase.lua().register_script("map", name, data);
	}

	// Now, read the global state if any is saved
	Widelands::FileRead fr;
	if (fr.TryOpen(fs, "scripting/globals.dump")) {
		egbase.lua().read_global_env(fr, mol, fr.Unsigned32());
	}
}


void Map_Scripting_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
throw (_wexception)
{

	ScriptContainer & p = egbase.lua().get_scripts_for("map");

	if (p.size())
		fs.EnsureDirectoryExists("scripting");

	for (ScriptContainer::iterator i = p.begin(); i != p.end(); i++) {
		std::string fname = "scripting/";
		fname += i->first;
		fname += ".lua";

		fs.Write(fname, i->second.c_str(), i->second.size());
	}

	// Dump the global environment
	Widelands::FileWrite fw;
	Widelands::FileWrite::Pos pos = fw.GetPos();
	fw.Unsigned32(0); // N bytes written, follows below

	uint32_t nwritten = Little32(egbase.lua().write_global_env(fw, mos));
	fw.Data(&nwritten, 4, pos);

	fw.Write(fs, "scripting/globals.dump");
}

}
