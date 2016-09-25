/*
 * Copyright (C) 2007-2016 by the Widelands Development Team
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
#include "profile/profile.h"
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
InteractiveSpectator::InteractiveSpectator(Widelands::Game& g,
                                           Section& global_s,
                                           bool const multiplayer)
   : InteractiveGameBase(g, global_s, OBSERVER, multiplayer, multiplayer) {
	toolbar_.set_layout_toplevel(true);
	if (is_multiplayer()) {
		toggle_options_menu_ = add_toolbar_button(
		   "wui/menus/menu_options_menu", "options_menu", _("Main Menu"), &options_);
		toggle_options_menu_->sigclicked.connect(
		   boost::bind(&InteractiveSpectator::toggle_options_menu, this));
	} else {
		exit_ = add_toolbar_button("wui/menus/menu_exit_game", "exit_replay", _("Exit Replay"));
		exit_->sigclicked.connect(boost::bind(&InteractiveSpectator::exit_btn, this));

		save_ = add_toolbar_button(
		   "wui/menus/menu_save_game", "save_game", _("Save Game"), &main_windows_.savegame);
		save_->sigclicked.connect(boost::bind(&InteractiveSpectator::save_btn, this));
	}
	toggle_statistics_ = add_toolbar_button("wui/menus/menu_general_stats", "general_stats",
	                                        _("Statistics"), &main_windows_.general_stats);
	toggle_statistics_->sigclicked.connect(
	   boost::bind(&InteractiveSpectator::toggle_statistics, this));

	toggle_minimap_ = add_toolbar_button(
	   "wui/menus/menu_toggle_minimap", "minimap", _("Minimap"), &minimap_registry());
	toggle_minimap_->sigclicked.connect(boost::bind(&InteractiveSpectator::toggle_minimap, this));

	toggle_buildhelp_ = add_toolbar_button(
	   "wui/menus/menu_toggle_buildhelp", "buildhelp", _("Show Building Spaces (on/off)"));
	toggle_buildhelp_->sigclicked.connect(boost::bind(&InteractiveBase::toggle_buildhelp, this));

	if (is_multiplayer()) {
		toggle_chat_ = add_toolbar_button("wui/menus/menu_chat", "chat", _("Chat"), &chat_);
		toggle_chat_->sigclicked.connect(boost::bind(&InteractiveSpectator::toggle_chat, this));
	}

	adjust_toolbar_position();

	// Setup all screen elements
	fieldclicked.connect(boost::bind(&InteractiveSpectator::node_action, this));
}

InteractiveSpectator::~InteractiveSpectator() {
	chat_.unassign_toggle_button();
	options_.unassign_toggle_button();
	main_windows_.general_stats.unassign_toggle_button();
	main_windows_.savegame.unassign_toggle_button();
	minimap_registry().unassign_toggle_button();
}

/**
 * \return "our" player.
 *
 * \note We might want to implement a feature to watch a specific player,
 * including their vision. Then this should be changed.
 */
Widelands::Player* InteractiveSpectator::get_player() const {
	return nullptr;
}

int32_t InteractiveSpectator::calculate_buildcaps(const Widelands::TCoords<Widelands::FCoords>& c) {
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
void InteractiveSpectator::toggle_chat() {
	if (chat_.window)
		delete chat_.window;
	else if (chat_provider_)
		GameChatMenu::create_chat_console(this, chat_, *chat_provider_);
}

void InteractiveSpectator::exit_btn() {
	if (is_multiplayer()) {
		return;
	}
	end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
}

void InteractiveSpectator::save_btn() {
	if (is_multiplayer()) {
		return;
	}
	if (main_windows_.savegame.window)
		delete main_windows_.savegame.window;
	else {
		new GameMainMenuSaveGame(*this, main_windows_.savegame);
	}
}

void InteractiveSpectator::toggle_options_menu() {
	if (!is_multiplayer()) {
		return;
	}
	if (options_.window)
		delete options_.window;
	else
		new GameOptionsMenu(*this, options_, main_windows_);
}

void InteractiveSpectator::toggle_statistics() {
	if (main_windows_.general_stats.window)
		delete main_windows_.general_stats.window;
	else
		new GeneralStatisticsMenu(*this, main_windows_.general_stats);
}

bool InteractiveSpectator::can_see(Widelands::PlayerNumber) const {
	return true;
}
bool InteractiveSpectator::can_act(Widelands::PlayerNumber) const {
	return false;
}
Widelands::PlayerNumber InteractiveSpectator::player_number() const {
	return 0;
}

/**
 * Observer has clicked on the given node; bring up the context menu.
 */
void InteractiveSpectator::node_action() {
	if  //  special case for buildings
	   (upcast(Widelands::Building, building, egbase().map().get_immovable(get_sel_pos().node)))
		return building->show_options(*this);

	if (try_show_ship_window())
		return;

	//  everything else can bring up the temporary dialog
	show_field_action(this, nullptr, &fieldaction_);
}

/**
 * Global in-game keypresses:
 */
bool InteractiveSpectator::handle_key(bool const down, SDL_Keysym const code) {
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
				new GameMainMenuSaveGame(*this, main_windows_.savegame);
			} else
				set_display_flag(dfShowStatistics, !get_display_flag(dfShowStatistics));
			return true;

		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			if (!chat_provider_ | !chatenabled_)
				break;

			if (!chat_.window)
				GameChatMenu::create_chat_console(this, chat_, *chat_provider_);

			dynamic_cast<GameChatMenu*>(chat_.window)->enter_chat_message();
			return true;

		default:
			break;
		}

	return InteractiveGameBase::handle_key(down, code);
}
