/*
 * Copyright (C) 2008-2010 by the Widelands Development Team
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

#ifndef PLAYERSMANAGER_H
#define PLAYERSMANAGER_H

#include <string>
#include <vector>

#include "constants.h"
#include "logic/widelands.h"

namespace Widelands {

class Editor_Game_Base;
class Player;
class Player;

enum class PlayerEndResult : uint8_t
	{PLAYER_LOST = 0, PLAYER_WON = 1, PLAYER_RESIGNED = 2, UNDEFINED = 255};

/**
 * Hold data once a player left the game, or on game ends.
 * Allowed values for the info string, as key=value pairs separated
 * by semi-comma (;) :
 * \e wc_name : The win condition name (string)
 * \e wc_version : The win condition version (uint8)
 * \e score : The player score (uint16)
 * \e resign_reason : The reason for resigning (forfeit, disconnection, ..) (string)
 */
struct PlayerEndStatus {
	Player_Number player;
	PlayerEndResult result;
	uint32_t time;
	std::string info;
};

class Players_Manager {
public:
	Players_Manager(Editor_Game_Base & egbase);
	virtual ~Players_Manager();

	void cleanup();

	void remove_player(Player_Number);

	/**
	 * Create the player structure for the given plnum.
	 * Note that AI player structures and the Interactive_Player are created when
	 * the game starts. Similar for remote players.
	*/
	Player * add_player
		(Player_Number,
		 uint8_t             initialization_index,
		 const std::string & tribe,
		 const std::string & name,
		 TeamNumber team = 0);
	Player * get_player(int32_t n) const {
		assert(1 <= n);
		assert     (n <= MAX_PLAYERS);
		return m_players[n - 1];
	}
	Player & player(int32_t n) const {
		assert(1 <= n);
		assert     (n <= MAX_PLAYERS);
		return *m_players[n - 1];
	}

	/**
	 * \return the number of players (human or ai)
	 */
	uint8_t get_number_of_players() {return m_number_of_players;}

	const std::vector<PlayerEndStatus> & get_players_end_status() {return m_players_end_status;}

	/**
	* Adds a new player status for a player that left the game.
	*/
	void add_player_end_status(const PlayerEndStatus & status);

private:
	Player                 * m_players[MAX_PLAYERS];
	Editor_Game_Base       & m_egbase;
	uint8_t                  m_number_of_players;
	std::vector<PlayerEndStatus> m_players_end_status;
};
}

#endif
