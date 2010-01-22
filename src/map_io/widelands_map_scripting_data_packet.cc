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
#include "widelands_map_scripting_data_packet.h"

#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/world.h"
#include "profile/profile.h"
#include "scripting/scripting.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1

void Map_Scripting_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool,
	 Map_Map_Object_Loader * const)
throw (_wexception)
{
	filenameset_t scripting_files;

	fs.FindFiles("scripting", "*.lua", &scripting_files);
	for
		(filenameset_t::iterator i = scripting_files.begin();
		 i != scripting_files.end(); i++)
	{

		size_t length;
		std::string data(static_cast<char *>(fs.Load(*i, length)));

		std::string name = i->substr(0, i->size() - 4); // strips '.lua'
		size_t pos = name.rfind('/');
		if (pos == std::string::npos)
			pos = name.rfind("\\");
		if (pos != std::string::npos)
			name = name.substr(pos + 1, name.size());

		egbase.lua().register_script("map", name, data);
	}
}


void Map_Scripting_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver * const)
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
}

}
