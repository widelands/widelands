/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#include "game.h"

/// \return a pointer to the running \ref Game instance.
Widelands::Game * Interactive_GameBase::get_game() const
{
	return dynamic_cast<Widelands::Game *>(&egbase());
}


Widelands::Game & Interactive_GameBase::    game() const
{
	return dynamic_cast<Widelands::Game &>(egbase());
}

void Interactive_GameBase::set_chat_provider(ChatProvider* chat)
{
	m_chatProvider = chat;
	m_chatDisplay->setChatProvider(chat);

	chatenabled = true;
}

ChatProvider* Interactive_GameBase::get_chat_provider()
{
	return m_chatProvider;
}