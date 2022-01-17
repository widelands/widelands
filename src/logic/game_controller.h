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

#ifndef WL_LOGIC_GAME_CONTROLLER_H
#define WL_LOGIC_GAME_CONTROLLER_H

#include <string>

#include "base/times.h"
#include "logic/player_end_result.h"

namespace Widelands {
class Game;
class PlayerCommand;
}  // namespace Widelands

/**
 * A game controller implements the policies surrounding the actual
 * game simulation. For example, the GameController decides where
 * player commands go and how fast the simulation runs.
 *
 * The idea is to empty the current \ref Game class from singleplayer
 * vs. multiplayer vs. replay issues and have a \ref GameController
 * handle all that.
 */
class GameController {
public:
	enum class GameType : uint8_t { kUndefined = 0, kSingleplayer, kNetClient, kNetHost, kReplay };

	virtual ~GameController() {
	}

	virtual void think() = 0;

	// TODO(Klaus Halfmann): Command must be deleted once it was handled.
	virtual void send_player_command(Widelands::PlayerCommand*) = 0;
	virtual Duration get_frametime() = 0;
	virtual GameType get_game_type() = 0;

	/**
	 * \return the effective speed, in milliseconds per second,
	 * that the game is running at.
	 */
	virtual uint32_t real_speed() = 0;

	/**
	 * \return the speed that the local player wants the game to run at.
	 */
	virtual uint32_t desired_speed() = 0;

	/**
	 * Indicate to the game controller the speed (in milliseconds per second)
	 * that the local player wants the game to run at.
	 */
	virtual void set_desired_speed(uint32_t speed) = 0;

	/**
	 * Whether the game is paused.
	 */
	virtual bool is_paused() = 0;

	/**
	 * Whether the game is stopped.
	 */
	bool is_paused_or_zero_speed() {
		return is_paused() || real_speed() == 0;
	}

	/**
	 * Sets whether the game is paused.
	 */
	virtual void set_paused(const bool paused) = 0;

	/**
	 * Toggle pause state (convenience function)
	 */
	void toggle_paused() {
		set_paused(!is_paused());
	}

	/**
	 * Report a player result once he has left the game. This may be done through lua
	 * by the win_condition scripts.
	 */
	virtual void report_result(uint8_t /* player */,
	                           Widelands::PlayerEndResult /*result*/,
	                           const std::string& /* info */) {
	}

	/** Callback when the game setup UI is closed before a game was started. */
	virtual void game_setup_aborted() {
	}
};

#endif  // end of include guard: WL_LOGIC_GAME_CONTROLLER_H
