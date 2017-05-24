/*
 * Copyright (C) 2007-2017 by the Widelands Development Team
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
                                           bool const multiplayer,
                                           ChatProvider* chat_provider)
   : InteractiveGameBase(g, global_s, OBSERVER, multiplayer, chat_provider) {
	if (is_multiplayer()) {
		add_toolbar_button(
		   "wui/menus/menu_options_menu", "options_menu", _("Main Menu"), &options_, true);
		options_.open_window = [this] { new GameOptionsMenu(*this, options_, main_windows_); };

	} else {
		UI::Button* button =
		   add_toolbar_button("wui/menus/menu_exit_game", "exit_replay", _("Exit Replay"));
		button->sigclicked.connect(boost::bind(&InteractiveSpectator::exit_btn, this));

		add_toolbar_button("wui/menus/menu_save_game", "save_game",
		                   /** TRANSLATORS: When watching a replay, one can save it as a playable
		                      game. This is the button tooltip. */
		                   _("Save as Game"),
		                   &main_windows_.savegame, true);
		main_windows_.savegame.open_window = [this] {
			new GameMainMenuSaveGame(*this, main_windows_.savegame);
		};
	}
	add_toolbar_button("wui/menus/menu_general_stats", "general_stats", _("Statistics"),
	                   &main_windows_.general_stats, true);
	main_windows_.general_stats.open_window = [this] {
		new GeneralStatisticsMenu(*this, main_windows_.general_stats);
	};

	toolbar_.add_space(15);

	add_toolbar_button(
	   "wui/menus/menu_toggle_minimap", "minimap", _("Minimap"), &minimap_registry(), true);
	minimap_registry().open_window = [this] { toggle_minimap(); };

	toggle_buildhelp_ = add_toolbar_button(
	   "wui/menus/menu_toggle_buildhelp", "buildhelp", _("Show Building Spaces (on/off)"));
	toggle_buildhelp_->sigclicked.connect(boost::bind(&InteractiveBase::toggle_buildhelp, this));

	reset_zoom_ = add_toolbar_button("wui/menus/menu_reset_zoom", "reset_zoom", _("Reset zoom"));
	reset_zoom_->sigclicked.connect([this] {
		zoom_around(1.f, Vector2f(get_w() / 2.f, get_h() / 2.f), MapView::Transition::Smooth);
	});

	toolbar_.add_space(15);

	if (is_multiplayer()) {
		add_toolbar_button("wui/menus/menu_chat", "chat", _("Chat"), &chat_, true);
		chat_.open_window = [this] {
			if (chat_provider_) {
				GameChatMenu::create_chat_console(this, chat_, *chat_provider_);
			}
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
	// Special case for buildings
	if (is_a(Widelands::Building, egbase().map().get_immovable(get_sel_pos().node))) {
		show_building_window(get_sel_pos().node, false);
		return;
	}

	if (try_show_ship_window()) {
		return;
	}

	//  everything else can bring up the temporary dialog
	show_field_action(this, nullptr, &fieldaction_);
}

/**
 * Global in-game keypresses:
 */
bool InteractiveSpectator::handle_key(bool const down, SDL_Keysym const code) {
	if (down) {
		switch (code.sym) {

		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			if (chat_provider_) {
				if (!chat_.window) {
					GameChatMenu::create_chat_console(this, chat_, *chat_provider_);
				}
				dynamic_cast<GameChatMenu*>(chat_.window)->enter_chat_message();
			}
			return true;
		default:
			break;
		}
	}

	return InteractiveGameBase::handle_key(down, code);
}
