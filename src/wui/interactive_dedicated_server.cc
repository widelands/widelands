/*
 * Copyright (C) 2009 by the Widelands Development Team
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

#include "interactive_dedicated_server.h"

#include "chat.h"
#include "game_chat_menu.h"
#include "game_main_menu_save_game.h"
#include "game_options_menu.h"
#include "gamecontroller.h"
#include "general_statistics_menu.h"
#include "graphic/graphic.h"
#include "i18n.h"

#include "ui_basic/editbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

/**
 * Setup the dedictated server UI for the given game.
 */
Interactive_DServer::Interactive_DServer
	(Widelands::Game & _game, Section & global_s)
	:
	Interactive_GameBase(_game, global_s, NONE, true),

#define INIT_BTN(picture, callback, tooltip)                                  \
 TOOLBAR_BUTTON_COMMON_PARAMETERS,                                            \
 g_gr->get_picture(PicMod_Game, "pics/" picture ".png"),                      \
 &Interactive_DServer::callback, *this,                                       \
 tooltip                                                                      \

	m_toggle_chat
		(INIT_BTN("menu_chat",           toggle_chat,         _("Chat"))),
	m_toggle_options_menu
		(INIT_BTN("menu_options_menu",   toggle_options_menu, _("Options"))),
	m_toggle_statistics
		(INIT_BTN("menu_general_stats",  toggle_statistics,   _("Statistics")))
{
	m_toolbar.add(&m_toggle_chat,            UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_options_menu,    UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_statistics,      UI::Box::AlignLeft);

	m_chatDisplay =
		new ChatDisplay(this, 10, 25, get_w() - 10, get_h() - 25);

	m_toolbar.resize();
	adjust_toolbar_position();
}


/// just for compatibility
Widelands::Player * Interactive_DServer::get_player() const throw ()
{
	return 0;
}



/// Called just before the game starts, after postload, init and gfxload
/// just for compatibility
void Interactive_DServer::start() {}


/// Toolbar button callback functions.
void Interactive_DServer::toggle_chat()
{
	if (m_chat.window)
		delete m_chat.window;
	else if (m_chatProvider)
		new GameChatMenu(this, m_chat, *m_chatProvider);
}


void Interactive_DServer::toggle_options_menu() {
	if (!m_chatenabled) //  == !multiplayer
		return;
	if (m_options.window)
		delete m_options.window;
	else
		new GameOptionsMenu(*this, m_options, m_mainm_windows);
}


void Interactive_DServer::toggle_statistics() {
	if (m_mainm_windows.general_stats.window)
		delete m_mainm_windows.general_stats.window;
	else
		new General_Statistics_Menu(*this, m_mainm_windows.general_stats);
}


bool Interactive_DServer::can_see(Widelands::Player_Number) const
{
	throw;
}
bool Interactive_DServer::can_act(Widelands::Player_Number) const
{
	throw;
}
Widelands::Player_Number Interactive_DServer::player_number() const
{
	throw;
}

/// just for compatibility
void Interactive_DServer::node_action() {}


/// Global in-game keypresses:
bool Interactive_DServer::handle_key(bool down, SDL_keysym code)
{
	if (down)
		switch (code.sym) {

		case SDLK_f:
			g_gr->toggle_fullscreen();
			return true;

		case SDLK_RETURN:
			if (!m_chatProvider | !m_chatenabled)
				break;

			if (!m_chat.window)
				new GameChatMenu(this, m_chat, *m_chatProvider);

			dynamic_cast<GameChatMenu &>(*m_chat.window).enter_chat_message();
			return true;

		default:
			break;
		}

	return Interactive_GameBase::handle_key(down, code);
}

