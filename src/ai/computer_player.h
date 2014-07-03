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

#ifndef COMPUTER_PLAYER_H
#define COMPUTER_PLAYER_H

#include <boost/noncopyable.hpp>

#include "logic/game.h"
#include "logic/notification.h"

/**
 * The generic interface to AI instances, or "computer players".
 *
 * Instances of actual AI implementation can be created via the
 * \ref Implementation interface.
 */
struct Computer_Player :
	boost::noncopyable,
	Widelands::NoteReceiver<Widelands::NoteImmovable>,
	Widelands::NoteReceiver<Widelands::NoteFieldPossession>
{
	Computer_Player(Widelands::Game &, const Widelands::Player_Number);

	virtual void think () = 0;

	virtual void receive(const Widelands::NoteImmovable &) override {}
	virtual void receive(const Widelands::NoteFieldPossession     &) override {}

	Widelands::Game & game() const {return m_game;}
	Widelands::Player_Number player_number() {return m_player_number;}

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
	static const ImplementationVector & getImplementations();

	/**
	 * Get the best matching implementation for this name.
	 */
	static const Implementation * getImplementation(const std::string & name);

private:
	Widelands::Game & m_game;
	Widelands::Player_Number const m_player_number;
};

#endif
