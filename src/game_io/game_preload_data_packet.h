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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef GAME_PRELOAD_DATA_PACKET_H
#define GAME_PRELOAD_DATA_PACKET_H

#include "game_data_packet.h"

#include <string>
#include <cstring>

namespace Widelands {

/**
 * This contains all the preload data needed to identify
 * a game for a user (for example in a listbox)
 */
struct Game_Preload_Data_Packet : public Game_Data_Packet {
	void Read (FileSystem &, Game &, Map_Map_Object_Loader * = 0);
	void Write(FileSystem &, Game &, Map_Map_Object_Saver  * = 0);

	char const * get_mapname() {return m_mapname.c_str();}
	std::string get_background() {return m_background;}
	uint32_t get_gametime() {return m_gametime;}
	uint32_t get_player_nr() {return m_player_nr;}

private:
	std::string m_mapname;
	std::string m_background;
	uint32_t m_gametime;
	uint8_t m_player_nr;
};

}

#endif
