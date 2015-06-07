/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#ifndef WL_GAME_IO_GAME_PRELOAD_PACKET_H
#define WL_GAME_IO_GAME_PRELOAD_PACKET_H

#include <ctime>
#include <cstring>
#include <string>

#include "game_io/game_data_packet.h"

#include "logic/game_controller.h"

namespace Widelands {

/**
 * This contains all the preload data needed to identify
 * a game for a user (for example in a listbox)
 */

struct GamePreloadPacket : public GameDataPacket {
	void read (FileSystem &, Game &, MapObjectLoader * = nullptr) override;
	void write(FileSystem &, Game &, MapObjectSaver  * = nullptr) override;

	char const * get_mapname()      {return m_mapname.c_str();}
	std::string get_background()    {return m_background;}
	std::string get_win_condition() {return m_win_condition;}
	uint32_t get_gametime() {return m_gametime;}
	uint8_t get_player_nr() {return m_player_nr;}
	std::string get_version() {return m_version;}

	uint8_t get_number_of_players() {return m_number_of_players;}
	std::string get_minimap_path() {return m_minimap_path;}

	time_t get_savetimestamp() {return m_savetimestamp;}
	GameController::GameType get_gametype() {return m_gametype;}

private:
	std::string m_minimap_path;
	std::string m_mapname;
	std::string m_background;
	std::string m_win_condition;
	uint32_t m_gametime;
	uint8_t  m_player_nr; // The local player idx
	uint8_t  m_number_of_players;
	std::string m_version;
	time_t   m_savetimestamp;
	GameController::GameType m_gametype;
};

}

#endif  // end of include guard: WL_GAME_IO_GAME_PRELOAD_PACKET_H
