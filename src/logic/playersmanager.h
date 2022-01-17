/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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

#include <cassert>

#include "base/times.h"
#include "graphic/playercolor.h"
#include "logic/player_end_result.h"
#include "logic/widelands.h"

namespace Widelands {

class EditorGameBase;
class Player;
class Player;

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
	Time time;
	std::string info;
};

class PlayersManager {
public:
	explicit PlayersManager(EditorGameBase& egbase);
	virtual ~PlayersManager();

	void cleanup();

	void remove_player(PlayerNumber);

	/**
	 * Create the player structure for the given plnum.
	 * Note that AI player structures and the InteractivePlayer are created when
	 * the game starts. Similar for remote players.
	 */
	Player* add_player(PlayerNumber,
	                   uint8_t initialization_index,
	                   const RGBColor&,
	                   const std::string& tribe,
	                   const std::string& name,
	                   TeamNumber team = 0);
	Player* get_player(int32_t n) const {
		assert(1 <= n);
		assert(n <= kMaxPlayers);
		return players_[n - 1];
	}
	const Player& player(int32_t n) const {
		assert(1 <= n);
		assert(n <= kMaxPlayers);
		return *players_[n - 1];
	}

	/**
	 * \return the number of players (human or ai)
	 */
	uint8_t get_number_of_players() const {
		return number_of_players_;
	}

	const std::vector<PlayerEndStatus>& get_players_end_status() {
		return players_end_status_;
	}

	/**
	 * Adds a new player status for a player that left the game.
	 */
	void add_player_end_status(const PlayerEndStatus& status);

	/**
	 * Changes an already existing player end status
	 */
	void set_player_end_status(const PlayerEndStatus& status);

private:
	Player* players_[kMaxPlayers];
	EditorGameBase& egbase_;
	uint8_t number_of_players_;
	std::vector<PlayerEndStatus> players_end_status_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_PLAYERSMANAGER_H
