/*
 * Copyright (C) 2007-2009 by the Widelands Development Team
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

#include "interactive_gamebase.h"

#include "profile/profile.h"

#include "logic/game.h"
Interactive_GameBase::Interactive_GameBase
	(Widelands::Game & _game, Section & global_s,
	 PlayerType pt, bool const chatenabled)
	:
	Interactive_Base(_game, global_s),
	m_building_census_format
		(global_s.get_string("building_census_format",       "%N")),
	m_building_statistics_format
		(global_s.get_string("building_statistics_format",   "%t")),
	m_building_tooltip_format
		(global_s.get_string("building_tooltip_format",      "%r")),
	m_building_window_title_format
		(global_s.get_string("building_window_title_format", "%A")),
	m_chatenabled(chatenabled),
	m_playertype(pt)
{}

/// \return a pointer to the running \ref Game instance.
Widelands::Game * Interactive_GameBase::get_game() const
{
	return dynamic_cast<Widelands::Game *>(&egbase());
}


Widelands::Game & Interactive_GameBase::    game() const
{
	return ref_cast<Widelands::Game, Widelands::Editor_Game_Base>(egbase());
}

void Interactive_GameBase::set_chat_provider(ChatProvider & chat)
{
	m_chatProvider = &chat;
	m_chatDisplay->setChatProvider(chat);

	m_chatenabled = true;
}

ChatProvider * Interactive_GameBase::get_chat_provider()
{
	return m_chatProvider;
}
