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
	if (is_multiplayer()) {
		add_toolbar_button(
		   "wui/menus/menu_options_menu", "options_menu", _("Main Menu"), &options_, true);
		options_.open_window = [this] { new GameOptionsMenu(*this, options_, main_windows_); };

	} else {
		UI::Button* button =
		   add_toolbar_button("wui/menus/menu_exit_game", "exit_replay", _("Exit Replay"));
		button->sigclicked.connect(boost::bind(&InteractiveSpectator::exit_btn, this));

		add_toolbar_button(
		   "wui/menus/menu_save_game", "save_game", _("Save Game"), &main_windows_.savegame, true);
		main_windows_.savegame.open_window = [this] {
			new GameMainMenuSaveGame(*this, main_windows_.savegame);
		};
	}
	add_toolbar_button("wui/menus/menu_general_stats", "general_stats", _("Statistics"),
	                   &main_windows_.general_stats, true);
	main_windows_.general_stats.open_window = [this] {
		new GeneralStatisticsMenu(*this, main_windows_.general_stats);
	};

	add_toolbar_button(
	   "wui/menus/menu_toggle_minimap", "minimap", _("Minimap"), &minimap_registry(), true);
	minimap_registry().open_window = [this] { open_minimap(); };

	toggle_buildhelp_ = add_toolbar_button(
	   "wui/menus/menu_toggle_buildhelp", "buildhelp", _("Show Building Spaces (on/off)"));
	toggle_buildhelp_->sigclicked.connect(boost::bind(&InteractiveBase::toggle_buildhelp, this));

	if (is_multiplayer()) {
		add_toolbar_button("wui/menus/menu_chat", "chat", _("Chat"), &chat_, true);
		chat_.open_window = [this] {
			GameChatMenu::create_chat_console(this, chat_, *chat_provider_);
		};
	}

	adjust_toolbar_position();

	// Setup all screen elements
	fieldclicked.connect(boost::bind(&InteractiveSpectator::node_action, this));
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
void InteractiveSpectator::exit_btn() {
	if (is_multiplayer()) {
		return;
	}
	end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
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
			minimap_registry().toggle();
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
