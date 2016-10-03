/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#include "wui/interactive_player.h"

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/type_traits.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "economy/flag.h"
#include "game_io/game_loader.h"
#include "logic/cmd_queue.h"
#include "logic/constants.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "profile/profile.h"
#include "ui_basic/unique_window.h"
#include "wui/building_statistics_menu.h"
#include "wui/debugconsole.h"
#include "wui/fieldaction.h"
#include "wui/game_chat_menu.h"
#include "wui/game_main_menu.h"
#include "wui/game_main_menu_save_game.h"
#include "wui/game_message_menu.h"
#include "wui/game_objectives_menu.h"
#include "wui/game_options_menu.h"
#include "wui/general_statistics_menu.h"
#include "wui/stock_menu.h"
#include "wui/tribal_encyclopedia.h"
#include "wui/ware_statistics_menu.h"

using Widelands::Building;
using Widelands::Map;

InteractivePlayer::InteractivePlayer(Widelands::Game& g,
                                     Section& global_s,
                                     Widelands::PlayerNumber const plyn,
                                     bool const multiplayer)
   : InteractiveGameBase(g, global_s, NONE, multiplayer, multiplayer),
     auto_roadbuild_mode_(global_s.get_bool("auto_roadbuild_mode", true)),
     flag_to_connect_(Widelands::Coords::null()),

// Chat is different, as chat_provider_ needs to be checked when toggling
// Minimap is different as it warps and stuff

#define INIT_BTN_this(picture, name, tooltip)                                                      \
	TOOLBAR_BUTTON_COMMON_PARAMETERS(name), g_gr->images().get("images/" picture ".png"), tooltip

#define INIT_BTN(picture, name, tooltip)                                                           \
	TOOLBAR_BUTTON_COMMON_PARAMETERS(name), g_gr->images().get("images/" picture ".png"), tooltip

     toggle_chat_(INIT_BTN_this("wui/menus/menu_chat", "chat", _("Chat"))),
     toggle_options_menu_(INIT_BTN("wui/menus/menu_options_menu", "options_menu", _("Main Menu"))),
     toggle_statistics_menu_(
        INIT_BTN("wui/menus/menu_toggle_menu", "statistics_menu", _("Statistics"))),
     toggle_objectives_(INIT_BTN("wui/menus/menu_objectives", "objectives", _("Objectives"))),
     toggle_minimap_(INIT_BTN_this("wui/menus/menu_toggle_minimap", "minimap", _("Minimap"))),
     toggle_message_menu_(
        INIT_BTN("wui/menus/menu_toggle_oldmessage_menu", "messages", _("Messages"))),
     toggle_help_(INIT_BTN("ui_basic/menu_help", "help", _("Tribal Encyclopedia")))

{
	toggle_chat_.sigclicked.connect(boost::bind(&InteractivePlayer::toggle_chat, this));
	toggle_options_menu_.sigclicked.connect(
	   boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(options_)));
	toggle_statistics_menu_.sigclicked.connect(
	   boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(statisticsmenu_)));
	toggle_objectives_.sigclicked.connect(
	   boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(objectives_)));
	toggle_minimap_.sigclicked.connect(boost::bind(&InteractivePlayer::toggle_minimap, this));
	toggle_message_menu_.sigclicked.connect(
	   boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(message_menu_)));
	toggle_help_.sigclicked.connect(
	   boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(encyclopedia_)));

	// TODO(unknown): instead of making unneeded buttons invisible after generation,
	// they should not at all be generated. -> implement more dynamic toolbar UI
	toolbar_.add(&toggle_options_menu_, UI::Align::kLeft);
	toolbar_.add(&toggle_statistics_menu_, UI::Align::kLeft);
	toolbar_.add(&toggle_minimap_, UI::Align::kLeft);
	toolbar_.add(&toggle_buildhelp_, UI::Align::kLeft);
	if (multiplayer) {
		toolbar_.add(&toggle_chat_, UI::Align::kLeft);
		toggle_chat_.set_visible(false);
		toggle_chat_.set_enabled(false);
	}

	toolbar_.add(&toggle_objectives_, UI::Align::kLeft);
	toolbar_.add(&toggle_message_menu_, UI::Align::kLeft);
	toolbar_.add(&toggle_help_, UI::Align::kLeft);

	set_player_number(plyn);
	fieldclicked.connect(boost::bind(&InteractivePlayer::node_action, this));

	adjust_toolbar_position();

#define INIT_BTN_HOOKS(registry, btn)                                                              \
	registry.on_create = std::bind(&UI::Button::set_perm_pressed, &btn, true);                      \
	registry.on_delete = std::bind(&UI::Button::set_perm_pressed, &btn, false);                     \
	if (registry.window)                                                                            \
		btn.set_perm_pressed(true);

	INIT_BTN_HOOKS(chat_, toggle_chat_)
	INIT_BTN_HOOKS(options_, toggle_options_menu_)
	INIT_BTN_HOOKS(statisticsmenu_, toggle_statistics_menu_)
	INIT_BTN_HOOKS(minimap_registry(), toggle_minimap_)
	INIT_BTN_HOOKS(objectives_, toggle_objectives_)
	INIT_BTN_HOOKS(message_menu_, toggle_message_menu_)
	INIT_BTN_HOOKS(encyclopedia_, toggle_help_)

	encyclopedia_.open_window = [this] {
		new TribalEncyclopedia(*this, encyclopedia_, &game().lua());
	};
	options_.open_window = [this] { new GameOptionsMenu(*this, options_, main_windows_); };
	statisticsmenu_.open_window = [this] {
		new GameMainMenu(*this, statisticsmenu_, main_windows_);
	};
	objectives_.open_window = [this] { new GameObjectivesMenu(this, objectives_); };
	message_menu_.open_window = [this] { new GameMessageMenu(*this, message_menu_); };
	main_windows_.stock.open_window = [this] { new StockMenu(*this, main_windows_.stock); };

#ifndef NDEBUG  //  only in debug builds
	addCommand("switchplayer", boost::bind(&InteractivePlayer::cmdSwitchPlayer, this, _1));
#endif
}

InteractivePlayer::~InteractivePlayer() {
#define DEINIT_BTN_HOOKS(registry, btn)                                                            \
	registry.on_create = 0;                                                                         \
	registry.on_delete = 0;

	DEINIT_BTN_HOOKS(chat_, toggle_chat_)
	DEINIT_BTN_HOOKS(options_, toggle_options_menu_)
	DEINIT_BTN_HOOKS(statisticsmenu_, toggle_statistics_menu_)
	DEINIT_BTN_HOOKS(minimap_registry(), toggle_minimap_)
	DEINIT_BTN_HOOKS(objectives_, toggle_objectives_)
	DEINIT_BTN_HOOKS(message_menu_, toggle_message_menu_)
	DEINIT_BTN_HOOKS(encyclopedia_, toggle_help_)
}

void InteractivePlayer::think() {
	InteractiveBase::think();

	if (flag_to_connect_) {
		Widelands::Field& field = egbase().map()[flag_to_connect_];
		if (upcast(Widelands::Flag const, flag, field.get_immovable())) {
			if (!flag->has_road() && !is_building_road())
				if (auto_roadbuild_mode_) {
					//  There might be a fieldaction window open, showing a button
					//  for roadbuilding. If that dialog remains open so that the
					//  button is clicked, we would enter roadbuilding mode while
					//  we are already in roadbuilding mode from the call below.
					//  That is not allowed. Therefore we must delete the
					//  fieldaction window before entering roadbuilding mode here.
					delete fieldaction_.window;
					fieldaction_.window = nullptr;
					warp_mouse_to_node(flag_to_connect_);
					set_sel_pos(Widelands::NodeAndTriangle<>(
					   flag_to_connect_,
					   Widelands::TCoords<>(flag_to_connect_, Widelands::TCoords<>::D)));
					start_build_road(flag_to_connect_, field.get_owned_by());
				}
			flag_to_connect_ = Widelands::Coords::null();
		}
	}
	if (is_multiplayer()) {
		toggle_chat_.set_visible(chatenabled_);
		toggle_chat_.set_enabled(chatenabled_);
	}
	{
		char const* msg_icon = "images/wui/menus/menu_toggle_oldmessage_menu.png";
		std::string msg_tooltip = _("Messages");
		if (uint32_t const nr_new_messages =
		       player().messages().nr_messages(Widelands::Message::Status::kNew)) {
			msg_icon = "images/wui/menus/menu_toggle_newmessage_menu.png";
			msg_tooltip =
			   (boost::format(ngettext("%u new message", "%u new messages", nr_new_messages)) %
			    nr_new_messages)
			      .str();
		}
		toggle_message_menu_.set_pic(g_gr->images().get(msg_icon));
		toggle_message_menu_.set_tooltip(msg_tooltip);
	}
}

void InteractivePlayer::popup_message(Widelands::MessageId const id,
                                      const Widelands::Message& message) {
	message_menu_.create();
	dynamic_cast<GameMessageMenu&>(*message_menu_.window).show_new_message(id, message);
}

//  Toolbar button callback functions.
void InteractivePlayer::toggle_chat() {
	if (chat_.window)
		delete chat_.window;
	else if (chat_provider_)
		GameChatMenu::create_chat_console(this, chat_, *chat_provider_);
}

bool InteractivePlayer::can_see(Widelands::PlayerNumber const p) const {
	return p == player_number() || player().see_all();
}
bool InteractivePlayer::can_act(Widelands::PlayerNumber const p) const {
	return p == player_number();
}
Widelands::PlayerNumber InteractivePlayer::player_number() const {
	return player_number_;
}

int32_t InteractivePlayer::calculate_buildcaps(const Widelands::TCoords<Widelands::FCoords>& c) {
	assert(get_player());
	return get_player()->get_buildcaps(c);
}

/// Player has clicked on the given node; bring up the context menu.
void InteractivePlayer::node_action() {
	const Map& map = egbase().map();
	if (1 < player().vision(Map::get_index(get_sel_pos().node, map.get_width()))) {
		// Special case for buildings
		if (upcast(Building, building, map.get_immovable(get_sel_pos().node)))
			if (can_see(building->owner().player_number()))
				return show_building_window(get_sel_pos().node);

		if (!is_building_road()) {
			if (try_show_ship_window())
				return;
		}

		// everything else can bring up the temporary dialog
		show_field_action(this, get_player(), &fieldaction_);
	}
}

/**
 * Global in-game keypresses:
 * \li Space: toggles buildhelp
 * \li i: show stock (inventory)
 * \li m: show minimap
 * \li o: show objectives window
 * \li c: toggle census
 * \li s: toggle building statistics
 * \li Home: go to starting position
 * \li PageUp/PageDown: change game speed
 * \li Pause: pauses the game
 * \li Return: write chat message
*/
bool InteractivePlayer::handle_key(bool const down, SDL_Keysym const code) {
	if (down) {
		switch (code.sym) {
		case SDLK_SPACE:
			toggle_buildhelp();
			return true;

		case SDLK_i:
			main_windows_.stock.toggle();
			return true;

		case SDLK_m:
			toggle_minimap();
			return true;

		case SDLK_n:
			message_menu_.toggle();
			return true;

		case SDLK_o:
			objectives_.toggle();
			return true;

		case SDLK_F1:
			encyclopedia_.toggle();
			return true;

		case SDLK_c:
			set_display_flag(dfShowCensus, !get_display_flag(dfShowCensus));
			return true;

		case SDLK_b:
			if (main_windows_.building_stats.window == nullptr) {
				new BuildingStatisticsMenu(*this, main_windows_.building_stats);
			} else {
				main_windows_.building_stats.toggle();
			}
			return true;

		case SDLK_s:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				new GameMainMenuSaveGame(*this, main_windows_.savegame);
			else
				set_display_flag(dfShowStatistics, !get_display_flag(dfShowStatistics));
			return true;

		case SDLK_KP_7:
			if (code.mod & KMOD_NUM)
				break;
		/* no break */
		case SDLK_HOME:
			move_view_to(game().map().get_starting_pos(player_number_));
			return true;

		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			if (!chat_provider_ | !chatenabled_ || !is_multiplayer())
				break;

			if (!chat_.window)
				GameChatMenu::create_chat_console(this, chat_, *chat_provider_);

			return true;
		default:
			break;
		}
	}

	return InteractiveGameBase::handle_key(down, code);
}

/**
 * Set the player and the visibility to this
 * player
 */
void InteractivePlayer::set_player_number(uint32_t const n) {
	player_number_ = n;
}

/**
 * Cleanup any game-related data before loading a new game
 * while a game is currently playing.
 */
void InteractivePlayer::cleanup_for_load() {
}

void InteractivePlayer::cmdSwitchPlayer(const std::vector<std::string>& args) {
	if (args.size() != 2) {
		DebugConsole::write("Usage: switchplayer <nr>");
		return;
	}

	int const n = atoi(args[1].c_str());
	if (n < 1 || n > MAX_PLAYERS || !game().get_player(n)) {
		DebugConsole::write(str(boost::format("Player #%1% does not exist.") % n));
		return;
	}

	DebugConsole::write(
	   str(boost::format("Switching from #%1% to #%2%.") % static_cast<int>(player_number_) % n));
	player_number_ = n;

	if (UI::UniqueWindow* const building_statistics_window = main_windows_.building_stats.window) {
		dynamic_cast<BuildingStatisticsMenu&>(*building_statistics_window).update();
	}
}
