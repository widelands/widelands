/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#ifndef __S__WIDELANDS_MAP_PLAYERS_VIEW_DATA_PACKET_H
#define __S__WIDELANDS_MAP_PLAYERS_VIEW_DATA_PACKET_H

#include "widelands_map_data_packet.h"

namespace Widelands {

/// For each player, its view of each node, edge and triangle that he has seen
/// but does not see currently. Information that he currently sees is not
/// loaded but filled in from the game state.
///
/// Also the player's view of the resources that are hidden in the ground.
///
/// This information can not be loaded before the terrains, roads, immovables
/// and players' vision maps are loaded. The vision maps are completely loaded
/// after Map_Bobdata_Data_Packet has been loaded.
struct Map_Players_View_Data_Packet : public Map_Data_Packet {
	virtual void Read
		(FileSystem       &,
		 Editor_Game_Base *,
		 const bool,
		 Map_Map_Object_Loader * = 0)
		throw (_wexception);
	void Write(FileSystem &, Editor_Game_Base *, Map_Map_Object_Saver * = 0)
		throw (_wexception);
};

};

#endif
