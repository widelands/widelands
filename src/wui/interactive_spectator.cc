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

#include "base/i18n.h"
#include "base/macros.h"
#include "chat/chat.h"
#include "graphic/graphic.h"
#include "logic/game_controller.h"
#include "logic/player.h"
#include "ui_basic/editbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "wui/fieldaction.h"
#include "wui/game_chat_menu.h"
#include "wui/game_main_menu_save_game.h"
#include "wui/game_options_menu.h"
#include "wui/general_statistics_menu.h"

/**
 * Setup the replay UI for the given game.
 */
InteractiveSpectator::InteractiveSpectator
	(Widelands::Game & _game, Section & global_s, bool const multiplayer)
	:
	InteractiveGameBase(_game, global_s, OBSERVER, multiplayer, multiplayer),

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
	m_toggle_chat.sigclicked.connect(boost::bind(&InteractiveSpectator::toggle_chat, this));
	m_exit.sigclicked.connect(boost::bind(&InteractiveSpectator::exit_btn, this));
	m_save.sigclicked.connect(boost::bind(&InteractiveSpectator::save_btn, this));
	m_toggle_options_menu.sigclicked.connect(boost::bind(&InteractiveSpectator::toggle_options_menu, this));
	m_toggle_statistics.sigclicked.connect(boost::bind(&InteractiveSpectator::toggle_statistics, this));
	m_toggle_minimap.sigclicked.connect(boost::bind(&InteractiveSpectator::toggle_minimap, this));

	m_toolbar.set_layout_toplevel(true);
	if (!is_multiplayer()) {
		m_toolbar.add(&m_exit,                UI::Box::AlignLeft);
		m_toolbar.add(&m_save,                UI::Box::AlignLeft);
	} else
		m_toolbar.add(&m_toggle_options_menu, UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_statistics,      UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_minimap,         UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_buildhelp,       UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_chat,            UI::Box::AlignLeft);

	// TODO(unknown): instead of making unneeded buttons invisible after generation,
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
	fieldclicked.connect(boost::bind(&InteractiveSpectator::node_action, this));

#define INIT_BTN_HOOKS(registry, btn)                                                              \
	registry.on_create = std::bind(&UI::Button::set_perm_pressed, &btn, true);                      \
	registry.on_delete = std::bind(&UI::Button::set_perm_pressed, &btn, false);                     \
	if (registry.window)                                                                            \
		btn.set_perm_pressed(true);

	INIT_BTN_HOOKS(m_chat, m_toggle_chat)
	INIT_BTN_HOOKS(m_options, m_toggle_options_menu)
	INIT_BTN_HOOKS(m_mainm_windows.general_stats, m_toggle_statistics)
	INIT_BTN_HOOKS(m_mainm_windows.savegame, m_save)
	INIT_BTN_HOOKS(minimap_registry(), m_toggle_minimap)

}

InteractiveSpectator::~InteractiveSpectator() {
	// We need to remove these callbacks because the opened window might
        // (theoretically) live longer than 'this' window, and thus the
        // buttons. The assertions are safeguards in case somewhere else in the
        // code someone would overwrite our hooks.

#define DEINIT_BTN_HOOKS(registry, btn)                                                            \
	registry.on_create = 0;                                                                         \
	registry.on_delete = 0;

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
Widelands::Player * InteractiveSpectator::get_player() const
{
	return nullptr;
}


int32_t InteractiveSpectator::calculate_buildcaps(const Widelands::TCoords<Widelands::FCoords> c) {
	const Widelands::PlayerNumber nr_players = game().map().get_nrplayers();

	iterate_players_existing(p, nr_players, game(), player) {
		const Widelands::NodeCaps nc = player->get_buildcaps(c);
		if (nc > Widelands::NodeCaps::CAPS_NONE) {
			return nc;
		}
	}

	return Widelands::NodeCaps::CAPS_NONE;
}


// Toolbar button callback functions.
void InteractiveSpectator::toggle_chat()
{
	if (m_chat.window)
		delete m_chat.window;
	else if (m_chatProvider)
		GameChatMenu::create_chat_console(this, m_chat, *m_chatProvider);
}


void InteractiveSpectator::exit_btn()
{
	if (is_multiplayer()) {
		return;
	}
	end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
}


void InteractiveSpectator::save_btn()
{
	if (is_multiplayer()) {
		return;
	}
	if (m_mainm_windows.savegame.window)
		delete m_mainm_windows.savegame.window;
	else {
		new GameMainMenuSaveGame(*this, m_mainm_windows.savegame);
	}
}


void InteractiveSpectator::toggle_options_menu() {
	if (!is_multiplayer()) {
		return;
	}
	if (m_options.window)
		delete m_options.window;
	else
		new GameOptionsMenu(*this, m_options, m_mainm_windows);
}


void InteractiveSpectator::toggle_statistics() {
	if (m_mainm_windows.general_stats.window)
		delete m_mainm_windows.general_stats.window;
	else
		new GeneralStatisticsMenu(*this, m_mainm_windows.general_stats);
}


bool InteractiveSpectator::can_see(Widelands::PlayerNumber) const
{
	return true;
}
bool InteractiveSpectator::can_act(Widelands::PlayerNumber) const
{
	return false;
}
Widelands::PlayerNumber InteractiveSpectator::player_number() const
{
	return 0;
}


/**
 * Observer has clicked on the given node; bring up the context menu.
 */
void InteractiveSpectator::node_action() {
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
bool InteractiveSpectator::handle_key(bool const down, SDL_Keysym const code)
{
	if (down)
		switch (code.sym) {
		case SDLK_SPACE:
			toggle_buildhelp();
			return true;

		case SDLK_m:
			toggle_minimap();
			return true;

		case SDLK_c:
			set_display_flag(dfShowCensus, !get_display_flag(dfShowCensus));
			return true;

		case SDLK_s:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
				new GameMainMenuSaveGame(*this, m_mainm_windows.savegame);
			} else
				set_display_flag
					(dfShowStatistics, !get_display_flag(dfShowStatistics));
			return true;

		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			if (!m_chatProvider | !m_chatenabled)
				break;

			if (!m_chat.window)
				GameChatMenu::create_chat_console(this, m_chat, *m_chatProvider);

			dynamic_cast<GameChatMenu*>(m_chat.window)->enter_chat_message();
			return true;

		default:
			break;
		}

	return InteractiveGameBase::handle_key(down, code);
}
