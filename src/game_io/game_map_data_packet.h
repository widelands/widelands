/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#ifndef GAME_MAP_DATA_PACKET_H
#define GAME_MAP_DATA_PACKET_H

#include "game_io/game_data_packet.h"

namespace Widelands {

struct Map_Saver;
struct WL_Map_Loader;

/*
 * This is just a wrapper around Map_Saver and Map_Loader
 */
struct Game_Map_Data_Packet : public Game_Data_Packet {
	Game_Map_Data_Packet() : m_mos(nullptr), m_mol(nullptr), m_wms(nullptr), m_wml(nullptr) {}
	virtual ~Game_Map_Data_Packet();


	/// Ensures that the world gets loaded but does not much more.
	void Read (FileSystem &, Game &, Map_Map_Object_Loader * = nullptr) override;

	void Read_Complete(Game &); ///  Loads the rest of the map.

	void Write(FileSystem &, Game &, Map_Map_Object_Saver  * = nullptr) override;

	Map_Map_Object_Saver  * get_map_object_saver () {return m_mos;}
	Map_Map_Object_Loader * get_map_object_loader() {return m_mol;}

private:
	Map_Map_Object_Saver  * m_mos;
	Map_Map_Object_Loader * m_mol;
	Map_Saver             * m_wms;
	WL_Map_Loader         * m_wml;
};

}

#endif
