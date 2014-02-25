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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "computer_player.h"

#include "ai/defaultai.h"

Computer_Player::Computer_Player
	(Widelands::Game & g, Widelands::Player_Number const pid)
	: m_game(g), m_player_number(pid)
{
}


struct EmptyAI : Computer_Player {
	EmptyAI(Widelands::Game & g, const Widelands::Player_Number pid)
	: Computer_Player(g, pid) {}

	void think() override {}

	struct EmptyAIImpl : Implementation {
		EmptyAIImpl() {name = _("None");}
		Computer_Player * instantiate
			(Widelands::Game & g, Widelands::Player_Number const pid) const override
		{
			return new EmptyAI(g, pid);
		}
	};

	static EmptyAIImpl implementation;
};

EmptyAI::EmptyAIImpl EmptyAI::implementation;

const Computer_Player::ImplementationVector &
Computer_Player::getImplementations()
{
	static std::vector<Computer_Player::Implementation const *> impls;

	if (impls.empty()) {
		impls.push_back(&DefaultAI::aggressiveImpl);
		impls.push_back(&DefaultAI::normalImpl);
		impls.push_back(&DefaultAI::defensiveImpl);
		impls.push_back(&EmptyAI::implementation);
	}

	return impls;
}

const Computer_Player::Implementation * Computer_Player::getImplementation
	(const std::string & name)
{
	const ImplementationVector & vec = getImplementations();

	container_iterate_const(ImplementationVector, vec, i)
		if ((*i.current)->name == name)
			return *i.current;

	return vec[0];
}
