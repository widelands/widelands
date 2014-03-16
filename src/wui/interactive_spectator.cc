/*
 * Copyright (C) 2007-2011 by the Widelands Development Team
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

#include "wui/interactive_spectator.h"

#include "chat.h"
#include "gamecontroller.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "ui_basic/editbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "upcast.h"
#include "wui/fieldaction.h"
#include "wui/game_chat_menu.h"
#include "wui/game_main_menu_save_game.h"
#include "wui/game_options_menu.h"
#include "wui/general_statistics_menu.h"

/**
 * Setup the replay UI for the given game.
 */
Interactive_Spectator::Interactive_Spectator
	(Widelands::Game & _game, Section & global_s, bool const multiplayer)
	:
	Interactive_GameBase(_game, global_s, OBSERVER, multiplayer, multiplayer),

#define INIT_BTN(picture, name, tooltip)                            \
 TOOLBAR_BUTTON_COMMON_PARAMETERS(name),                                      \
 g_gr->images().get("pics/" picture ".png"),                      \
 tooltip                                                                      \

	m_toggle_chat
		(INIT_BTN("menu_chat", "chat", _("Chat"))),
	m_exit
		(INIT_BTN("menu_exit_game", "exit_replay", _("Exit Replay"))),
	m_save
		(INIT_BTN("menu_save_game", "save_game", _("Save Game"))),
	m_toggle_options_menu
		(INIT_BTN("menu_options_menu", "options_menu", _("Options"))),
	m_toggle_statistics
		(INIT_BTN("menu_general_stats", "general_stats", _("Statistics"))),
	m_toggle_minimap
		(INIT_BTN("menu_toggle_minimap", "minimap", _("Minimap")))
{
	m_toggle_chat.sigclicked.connect(boost::bind(&Interactive_Spectator::toggle_chat, this));
	m_exit.sigclicked.connect(boost::bind(&Interactive_Spectator::exit_btn, this));
	m_save.sigclicked.connect(boost::bind(&Interactive_Spectator::save_btn, this));
	m_toggle_options_menu.sigclicked.connect(boost::bind(&Interactive_Spectator::toggle_options_menu, this));
	m_toggle_statistics.sigclicked.connect(boost::bind(&Interactive_Spectator::toggle_statistics, this));
	m_toggle_minimap.sigclicked.connect(boost::bind(&Interactive_Spectator::toggle_minimap, this));

	m_toolbar.set_layout_toplevel(true);
	if (!is_multiplayer()) {
		m_toolbar.add(&m_exit,                UI::Box::AlignLeft);
		m_toolbar.add(&m_save,                UI::Box::AlignLeft);
	} else
		m_toolbar.add(&m_toggle_options_menu, UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_statistics,      UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_minimap,         UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_chat,            UI::Box::AlignLeft);

	// TODO : instead of making unneeded buttons invisible after generation,
	// they should not at all be generated. -> implement more dynamic toolbar UI
	if (is_multiplayer()) {
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

	adjust_toolbar_position();

	// Setup all screen elements
	fieldclicked.connect(boost::bind(&Interactive_Spectator::node_action, this));

	set_display_flag(dfSpeed, true);

#define INIT_BTN_HOOKS(registry, btn)                                        \
 registry.onCreate = boost::bind(&UI::Button::set_perm_pressed, &btn, true);  \
 registry.onDelete = boost::bind(&UI::Button::set_perm_pressed, &btn, false); \
 if (registry.window) btn.set_perm_pressed(true);                            \

	INIT_BTN_HOOKS(m_chat, m_toggle_chat)
	INIT_BTN_HOOKS(m_options, m_toggle_options_menu)
	INIT_BTN_HOOKS(m_mainm_windows.general_stats, m_toggle_statistics)
	INIT_BTN_HOOKS(m_mainm_windows.savegame, m_save)
	INIT_BTN_HOOKS(minimap_registry(), m_toggle_minimap)

}

Interactive_Spectator::~Interactive_Spectator() {
	// We need to remove these callbacks because the opened window might
        // (theoretically) live longer than 'this' window, and thus the
        // buttons. The assertions are safeguards in case somewhere else in the
        // code someone would overwrite our hooks.

#define DEINIT_BTN_HOOKS(registry, btn)                                                \
 assert (registry.onCreate == boost::bind(&UI::Button::set_perm_pressed, &btn, true));  \
 assert (registry.onDelete == boost::bind(&UI::Button::set_perm_pressed, &btn, false)); \
 registry.onCreate = 0;                                                                \
 registry.onDelete = 0;                                                                \

	DEINIT_BTN_HOOKS(m_chat, m_toggle_chat)
	DEINIT_BTN_HOOKS(m_options, m_toggle_options_menu)
	DEINIT_BTN_HOOKS(m_mainm_windows.general_stats, m_toggle_statistics)
	DEINIT_BTN_HOOKS(m_mainm_windows.savegame, m_save)
	DEINIT_BTN_HOOKS(minimap_registry(), m_toggle_minimap)
}


/**
 * \return "our" player.
 *
 * \note We might want to implement a feature to watch a specific player,
 * including their vision. Then this should be changed.
 */
Widelands::Player * Interactive_Spectator::get_player() const
{
	return nullptr;
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
	if (is_multiplayer()) {
		return;
	}
	end_modal(0);
}


void Interactive_Spectator::save_btn()
{
	if (is_multiplayer()) {
		return;
	}
	if (m_mainm_windows.savegame.window)
		delete m_mainm_windows.savegame.window;
	else {
		new Game_Main_Menu_Save_Game(*this, m_mainm_windows.savegame);
	}
}


void Interactive_Spectator::toggle_options_menu() {
	if (!is_multiplayer()) {
		return;
	}
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


bool Interactive_Spectator::can_see(Widelands::Player_Number) const
{
	return true;
}
bool Interactive_Spectator::can_act(Widelands::Player_Number) const
{
	return false;
}
Widelands::Player_Number Interactive_Spectator::player_number() const
{
	return 0;
}


/**
 * Observer has clicked on the given node; bring up the context menu.
 */
void Interactive_Spectator::node_action() {
	if //  special case for buildings
		(upcast
		 	(Widelands::Building,
		 	 building,
		 	 egbase().map().get_immovable(get_sel_pos().node)))
		return building->show_options(*this);

	if (try_show_ship_window())
		return;

	//  everything else can bring up the temporary dialog
	show_field_action(this, nullptr, &m_fieldaction);
}


/**
 * Global in-game keypresses:
 */
bool Interactive_Spectator::handle_key(bool const down, SDL_keysym const code)
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
				new Game_Main_Menu_Save_Game(*this, m_mainm_windows.savegame);
			} else
				set_display_flag
					(dfShowStatistics, !get_display_flag(dfShowStatistics));
			return true;

		case SDLK_f:
			g_gr->toggle_fullscreen();
			return true;

		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			if (!m_chatProvider | !m_chatenabled)
				break;

			if (!m_chat.window)
				new GameChatMenu(this, m_chat, *m_chatProvider);

			ref_cast<GameChatMenu, UI::UniqueWindow>(*m_chat.window)
				.enter_chat_message();
			return true;

		default:
			break;
		}

	return Interactive_GameBase::handle_key(down, code);
}
