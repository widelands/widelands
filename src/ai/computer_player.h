/*
 * Copyright (C) 2004, 2006-2011 by the Widelands Development Team
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

#ifndef WL_AI_COMPUTER_PLAYER_H
#define WL_AI_COMPUTER_PLAYER_H

#include <string>
#include <vector>

#include "base/macros.h"
#include "logic/widelands.h"

namespace Widelands {
class Game;
}  // namespace Widelands

/**
 * The generic interface to AI instances, or "computer players".
 *
 * Instances of actual AI implementation can be created via the
 * \ref Implementation interface.
 */
struct ComputerPlayer {
	ComputerPlayer(Widelands::Game &, const Widelands::PlayerNumber);
	virtual ~ComputerPlayer();

	virtual void think () = 0;

	Widelands::Game & game() const {return m_game;}
	Widelands::PlayerNumber player_number() {return m_player_number;}

	/**
	 * Interface to a concrete implementation, used to instantiate AIs.
	 *
	 * \see get_implementations()
	 */
	struct Implementation {
		std::string name;
		std::string descname;
		std::string icon_filename;
		virtual ~Implementation() {}
		virtual ComputerPlayer * instantiate
			(Widelands::Game &, Widelands::PlayerNumber) const = 0;
	};
	using ImplementationVector = std::vector<ComputerPlayer::Implementation const *>;

	/**
	 * Get a list of available AI implementations.
	 */
	static const ImplementationVector & get_implementations();

	/**
	 * Get the best matching implementation for this name.
	 */
	static const Implementation * get_implementation(const std::string & name);

private:
	Widelands::Game & m_game;
	Widelands::PlayerNumber const m_player_number;

	DISALLOW_COPY_AND_ASSIGN(ComputerPlayer);
};

#endif  // end of include guard: WL_AI_COMPUTER_PLAYER_H
