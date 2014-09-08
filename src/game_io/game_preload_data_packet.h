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

#ifndef WL_GAME_IO_GAME_PRELOAD_DATA_PACKET_H
#define WL_GAME_IO_GAME_PRELOAD_DATA_PACKET_H

#include <cstring>
#include <string>

#include "game_io/game_data_packet.h"

#include "logic/game_controller.h"

namespace Widelands {

/**
 * This contains all the preload data needed to identify
 * a game for a user (for example in a listbox)
 */
struct Game_Preload_Data_Packet : public Game_Data_Packet {

	void Read (FileSystem &, Game &, MapMapObjectLoader * = nullptr) override;
	void Write(FileSystem &, Game &, MapMapObjectSaver  * = nullptr) override;

	char const * get_mapname()      {return m_mapname.c_str();}
	std::string get_background()    {return m_background;}
	std::string get_win_condition() {return m_win_condition;}
	uint32_t get_gametime() {return m_gametime;}
	uint8_t get_player_nr() {return m_player_nr;}

	uint8_t get_number_of_players() {return m_number_of_players;}
	std::string get_minimap_path() {return m_minimap_path;}

	std::string get_localized_display_title(std::string filename);
	uint16_t get_saveyear() {return m_saveyear;}
	uint8_t get_savemonth() {return m_savemonth;}
	uint8_t get_saveday() {return m_saveday;}
	uint8_t get_savehour() {return m_savehour;}
	uint8_t get_saveminute() {return m_saveminute;}
	GameController::GameType get_gametype() {return m_gametype;}

private:
	std::string m_minimap_path;
	std::string m_mapname;
	std::string m_background;
	std::string m_win_condition;
	uint32_t m_gametime;
	uint8_t  m_player_nr; // The local player idx
	uint8_t  m_number_of_players;
	uint16_t m_saveyear;
	uint8_t  m_savemonth;
	uint8_t  m_saveday;
	uint8_t  m_savehour;
	uint8_t  m_saveminute;
	GameController::GameType m_gametype;
};

}

#endif  // end of include guard: WL_GAME_IO_GAME_PRELOAD_DATA_PACKET_H
