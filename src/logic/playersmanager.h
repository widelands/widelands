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

#ifndef WL_LOGIC_PLAYERSMANAGER_H
#define WL_LOGIC_PLAYERSMANAGER_H

#include <string>
#include <vector>

#include "logic/constants.h"
#include "logic/widelands.h"

namespace Widelands {

class EditorGameBase;
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
	PlayerNumber player;
	PlayerEndResult result;
	uint32_t time;
	std::string info;
};

class PlayersManager {
public:
	PlayersManager(EditorGameBase & egbase);
	virtual ~PlayersManager();

	void cleanup();

	void remove_player(PlayerNumber);

	/**
	 * Create the player structure for the given plnum.
	 * Note that AI player structures and the InteractivePlayer are created when
	 * the game starts. Similar for remote players.
	*/
	Player * add_player
		(PlayerNumber,
		 uint8_t             initialization_index,
		 const std::string & tribe,
		 const std::string & name,
		 TeamNumber team = 0);
	Player * get_player(int32_t n) const {
		assert(1 <= n);
		assert     (n <= MAX_PLAYERS);
		return players_[n - 1];
	}
	Player & player(int32_t n) const {
		assert(1 <= n);
		assert     (n <= MAX_PLAYERS);
		return *players_[n - 1];
	}

	/**
	 * \return the number of players (human or ai)
	 */
	uint8_t get_number_of_players() {return number_of_players_;}

	const std::vector<PlayerEndStatus> & get_players_end_status() {return players_end_status_;}

	/**
	* Adds a new player status for a player that left the game.
	*/
	void add_player_end_status(const PlayerEndStatus & status);

private:
	Player* players_[MAX_PLAYERS];
	EditorGameBase& egbase_;
	uint8_t number_of_players_;
	std::vector<PlayerEndStatus> players_end_status_;
};
}

#endif  // end of include guard: WL_LOGIC_PLAYERSMANAGER_H
