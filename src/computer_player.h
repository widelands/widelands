/*
 * Copyright (C) 2004, 2006-2009 by the Widelands Development Team
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

#ifndef COMPUTER_PLAYER_H
#define COMPUTER_PLAYER_H

#include "game.h"
#include "notification.h"

/**
 * The generic interface to AI instances, or "computer players".
 *
 * Instances of actual AI implementation can be created via the
 * \ref Implementation interface.
 */
struct Computer_Player : Widelands::NoteReceiver<Widelands::NoteImmovable>, Widelands::NoteReceiver<Widelands::NoteField> {
	Computer_Player(Widelands::Game &, const Widelands::Player_Number);
	~Computer_Player ();

	virtual void think () = 0;

	virtual void receive(Widelands::NoteImmovable const &) {}
	virtual void receive(Widelands::NoteField     const &) {}

	Widelands::Game & game() const throw () {return m_game;}
	Widelands::Player_Number get_player_number() {return player_number;}

	/**
	 * Interface to a concrete implementation, used to instantiate AIs.
	 *
	 * \see getImplementations()
	 */
	struct Implementation {
		std::string name;
		virtual ~Implementation() {}
		virtual Computer_Player * instantiate
			(Widelands::Game &, Widelands::Player_Number) const = 0;
	};
	typedef
		std::vector<Computer_Player::Implementation const *>
		ImplementationVector;

	/**
	 * Get a list of available AI implementations.
	 */
	static ImplementationVector const & getImplementations();

	/**
	 * Get the best matching implementation for this name.
	 */
	static const Implementation * getImplementation(const std::string & name);

private:
	Widelands::Game & m_game;
	Widelands::Player_Number const player_number;
};

#endif
