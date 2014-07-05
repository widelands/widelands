/*
 * Copyright (C) 2008-2011, 2013, 2014 by the Widelands Development Team
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

#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <string>

#include "logic/widelands.h"

namespace Widelands {
class Game;
class PlayerCommand;
enum class PlayerEndResult: uint8_t;
}


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
	virtual ~GameController() {}

	virtual void think() = 0;
	virtual void sendPlayerCommand(Widelands::PlayerCommand &) = 0;
	virtual int32_t getFrametime() = 0;
	virtual std::string getGameDescription() = 0;

	/**
	 * \return the effective speed, in milliseconds per second,
	 * that the game is running at.
	 */
	virtual uint32_t realSpeed() = 0;

	/**
	 * \return the speed that the local player wants the game to run at.
	 */
	virtual uint32_t desiredSpeed() = 0;

	/**
	 * Indicate to the game controller the speed (in milliseconds per second)
	 * that the local player wants the game to run at.
	 */
	virtual void setDesiredSpeed(uint32_t speed) = 0;

	/**
	 * Whether the game is paused.
	 */
	virtual bool isPaused() = 0;

	/**
	 * Sets whether the game is paused.
	 */
	virtual void setPaused(const bool paused) = 0;

	/**
	 * Toggle pause state (convenience function)
	 */
	void togglePaused() {
		setPaused(not isPaused());
	}

	/**
	 * Report a player result once he has left the game. This may be done through lua
	 * by the win_condition scripts.
	 * \param player : the player idx;
	 * \param result : the player result
	 * \param info : The info string (\see \struct PlayerEndStatus for allowed values)
	 */
	virtual void report_result
	    (uint8_t /* player */,
	     Widelands::PlayerEndResult /*result*/,
	     const std::string & /* info */)
	{}
};

#endif
