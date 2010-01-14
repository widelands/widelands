/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#include "widelands_map_node_ownership_data_packet.h"

#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/world.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


void Map_Node_Ownership_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader &)
	throw (_wexception)
{
	if (skip)
		return;
	FileRead fr;
	try {
		fr.Open(fs, "binary/node_ownership");
	} catch (...) {
		try {
			fr.Open(fs, "binary/owned_fields");
		} catch (...) {
			return;
		}
	}
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			Map & map = egbase.map();
			Map_Index const max_index = map.max_index();
			for (Map_Index i = 0; i < max_index; ++i)
				map[i].set_owned_by(fr.Unsigned8());
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("ownership: %s"), e.what());
	}
}


void Map_Node_Ownership_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver &)
throw (_wexception)
{
	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	Map & map = egbase.map();
	Map_Index const max_index = map.max_index();
	for (Map_Index i = 0; i < max_index; ++i)
		fw.Unsigned8(map[i].get_owned_by());

	fw.Write(fs, "binary/node_ownership");
}

}
