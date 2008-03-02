/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include "widelands.h"

#include <string>

namespace Widelands {
class PlayerCommand;
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
protected:
	virtual ~GameController() {}

public:
	virtual void think() = 0;
	virtual void sendPlayerCommand(Widelands::PlayerCommand* pc) = 0;
	virtual int32_t getFrametime() = 0;
	virtual std::string getGameDescription() = 0;
};

#endif // GAMECONTROLLER_H
