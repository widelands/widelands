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

#include "interactive_spectator.h"

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
 * Setup the replay UI for the given game.
 */
Interactive_Spectator::Interactive_Spectator
	(Widelands::Game & _game, Section & global_s, bool const multiplayer)
	:
	Interactive_GameBase(_game, global_s, OBSERVER, multiplayer),

#define INIT_BTN(picture, callback, tooltip)                                  \
 TOOLBAR_BUTTON_COMMON_PARAMETERS,                                            \
 g_gr->get_picture(PicMod_Game, "pics/" picture ".png"),                      \
 &Interactive_Spectator::callback, *this,                                     \
 tooltip                                                                      \

	m_toggle_chat
		(INIT_BTN("menu_chat",           toggle_chat,         _("Chat"))),
	m_exit
		(INIT_BTN("menu_exit_game",      exit_btn,            _("Exit Replay"))),
	m_save
		(INIT_BTN("menu_save_game",      save_btn,            _("Save Game"))),
	m_toggle_options_menu
		(INIT_BTN("menu_options_menu",   toggle_options_menu, _("Options"))),
	m_toggle_statistics
		(INIT_BTN("menu_general_stats",  toggle_statistics,   _("Statistics"))),
	m_toggle_minimap
		(INIT_BTN("menu_toggle_minimap", toggle_minimap,      _("Minimap")))
{
	m_toolbar.add(&m_toggle_chat,            UI::Box::AlignLeft);
	if (!multiplayer) {
		m_toolbar.add(&m_exit,                UI::Box::AlignLeft);
		m_toolbar.add(&m_save,                UI::Box::AlignLeft);
	} else
		m_toolbar.add(&m_toggle_options_menu, UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_statistics,      UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_minimap,         UI::Box::AlignLeft);

	// TODO : instead of making unneeded buttons invisible after generation,
	// they should not at all be generated. -> implement more dynamic toolbar UI
	if (multiplayer) {
		m_chatDisplay =
			new ChatDisplay(this, 10, 25, get_w() - 10, get_h() - 25);
		m_exit.set_visible(false);
		m_exit.set_enabled(false);
		m_save.set_visible(false);
		m_save.set_enabled(false);
	} else {
		m_toggle_chat.set_visible(false);
		m_toggle_chat.set_enabled(false);
		m_toggle_options_menu.set_visible(false);
		m_toggle_options_menu.set_enabled(false);
	}

	m_toolbar.resize();
	adjust_toolbar_position();

	// Setup all screen elements
	fieldclicked.set(this, &Interactive_Spectator::node_action);

	set_display_flag(dfSpeed, true);
}


/**
 * \return "our" player.
 *
 * \note We might want to implement a feature to watch a specific player,
 * including their vision. Then this should be changed.
 */
Widelands::Player * Interactive_Spectator::get_player() const throw ()
{
	return 0;
}


/**
 * Called just before the game starts, after postload, init and gfxload
 */
void Interactive_Spectator::start()
{
	Widelands::Map & map = game().map();
	Overlay_Manager & overlay_manager = map.overlay_manager();
	overlay_manager.show_buildhelp(false);

	// Recalc whole map for changed owner stuff
	map.recalc_whole_map();
}


// Toolbar button callback functions.
void Interactive_Spectator::toggle_chat()
{
	if (m_chat.window)
		delete m_chat.window;
	else if (m_chatProvider)
		new GameChatMenu(this, m_chat, *m_chatProvider);
}


void Interactive_Spectator::exit_btn()
{
	if (m_chatenabled) //  == multiplayer
		return;
	end_modal(0);
}


void Interactive_Spectator::save_btn()
{
	if (m_chatenabled) //  == multiplayer
		return;
	if (m_mainm_windows.savegame.window)
		delete m_mainm_windows.savegame.window;
	else {
		game().gameController()->setDesiredSpeed(0);
		new Game_Main_Menu_Save_Game(*this, m_mainm_windows.savegame);
	}
}


void Interactive_Spectator::toggle_options_menu() {
	if (!m_chatenabled) //  == !multiplayer
		return;
	if (m_options.window)
		delete m_options.window;
	else
		new GameOptionsMenu(*this, m_options, m_mainm_windows);
}


void Interactive_Spectator::toggle_statistics() {
	if (m_mainm_windows.general_stats.window)
		delete m_mainm_windows.general_stats.window;
	else
		new General_Statistics_Menu(*this, m_mainm_windows.general_stats);
}


/**
 * Observer has clicked on the given field; bring up the context menu.
 * \todo Implement
 */
void Interactive_Spectator::node_action() {}


/**
 * Global in-game keypresses:
 */
bool Interactive_Spectator::handle_key(bool down, SDL_keysym code)
{
	if (down)
		switch (code.sym) {
		case SDLK_m:
			toggle_minimap();
			return true;

		case SDLK_c:
			set_display_flag(dfShowCensus, !get_display_flag(dfShowCensus));
			return true;

		case SDLK_s:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
				game().gameController()->setDesiredSpeed(0);
				new Game_Main_Menu_Save_Game(*this, m_mainm_windows.savegame);
			} else
				set_display_flag
					(dfShowStatistics, !get_display_flag(dfShowStatistics));
			return true;

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

