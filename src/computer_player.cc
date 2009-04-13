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

#include "computer_player.h"

#include "ai/defaultai.h"

Computer_Player::Computer_Player(Widelands::Game & g, const Widelands::Player_Number pid)
	: m_game(g), player_number(pid)
{
}


#ifdef DEBUG
struct EmptyAI : Computer_Player {
	EmptyAI(Widelands::Game & g, const Widelands::Player_Number pid)
	: Computer_Player(g, pid) {}

	void think() {}

	struct EmptyAIImpl : Implementation {
		EmptyAIImpl() {name = "none";}
		Computer_Player * instantiate
			(Widelands::Game & g,
			 const Widelands::Player_Number pid) const
		{
			return new EmptyAI(g, pid);
		}
	};

	static EmptyAIImpl implementation;
};

EmptyAI::EmptyAIImpl EmptyAI::implementation;
#endif

Computer_Player::ImplementationVector const &
Computer_Player::getImplementations()
{
	static std::vector<Computer_Player::Implementation const *> impls;

	if (impls.empty()) {
		impls.push_back(&DefaultAI::implementation);
#ifdef DEBUG
		impls.push_back(&EmptyAI::implementation);
#endif
	}

	return impls;
}

const Computer_Player::Implementation * Computer_Player::getImplementation
	(const std::string & name)
{
	ImplementationVector const & vec = getImplementations();

	container_iterate_const(ImplementationVector, vec, i)
		if ((*i.current)->name == name)
			return *i.current;

	return vec[0];
}
