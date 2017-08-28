/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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
#include "wui/game_main_menu_save_game.h"
#include "wui/game_message_menu.h"
#include "wui/game_objectives_menu.h"
#include "wui/game_options_menu.h"
#include "wui/game_statistics_menu.h"
#include "wui/general_statistics_menu.h"
#include "wui/stock_menu.h"
#include "wui/tribal_encyclopedia.h"
#include "wui/ware_statistics_menu.h"

using Widelands::Building;
using Widelands::Map;

namespace  {

// Returns the brightness value in [0, 1.] for 'fcoords' at 'gametime' for
// 'pf'. See 'field_brightness' in fields_to_draw.cc for scale of values.
float adjusted_field_brightness(const Widelands::FCoords& fcoords,
                       const uint32_t gametime,
                       const Widelands::Player::Field& pf) {
	if (pf.vision == 0) {
		return 0.;
	};

	uint32_t brightness = 144 + fcoords.field->get_brightness();
	brightness = std::min<uint32_t>(255, (brightness * 255) / 160);

	if (pf.vision == 1) {
		static const uint32_t kDecayTimeInMs = 20000;
		const Widelands::Duration time_ago = gametime - pf.time_node_last_unseen;
		if (time_ago < kDecayTimeInMs) {
			brightness = (brightness * (2 * kDecayTimeInMs - time_ago)) / (2 * kDecayTimeInMs);
		} else {
			brightness = brightness / 2;
		}
	}
	return brightness / 255.;
}

}  // namespace 

InteractivePlayer::InteractivePlayer(Widelands::Game& g,
                                     Section& global_s,
                                     Widelands::PlayerNumber const plyn,
                                     bool const multiplayer)
   : InteractiveGameBase(g, global_s, NONE, multiplayer),
     auto_roadbuild_mode_(global_s.get_bool("auto_roadbuild_mode", true)),
     flag_to_connect_(Widelands::Coords::null()) {
	add_toolbar_button(
	   "wui/menus/menu_options_menu", "options_menu", _("Main Menu"), &options_, true);
	options_.open_window = [this] { new GameOptionsMenu(*this, options_, main_windows_); };

	add_toolbar_button(
	   "wui/menus/menu_toggle_menu", "statistics_menu", _("Statistics"), &statisticsmenu_, true);
	statisticsmenu_.open_window = [this] {
		new GameStatisticsMenu(*this, statisticsmenu_, main_windows_);
	};

	toolbar()->add_space(15);

	add_toolbar_button(
	   "wui/menus/menu_toggle_minimap", "minimap", _("Minimap"), &minimap_registry(), true);
	minimap_registry().open_window = [this] { toggle_minimap(); };

	toggle_buildhelp_ = add_toolbar_button(
	   "wui/menus/menu_toggle_buildhelp", "buildhelp", _("Show Building Spaces (on/off)"));
	toggle_buildhelp_->sigclicked.connect(boost::bind(&InteractiveBase::toggle_buildhelp, this));
	reset_zoom_ = add_toolbar_button("wui/menus/menu_reset_zoom", "reset_zoom", _("Reset zoom"));
	reset_zoom_->sigclicked.connect([this] {
		map_view()->zoom_around(
		   1.f, Vector2f(get_w() / 2.f, get_h() / 2.f), MapView::Transition::Smooth);
	});
	toolbar()->add_space(15);
	if (multiplayer) {
		toggle_chat_ = add_toolbar_button("wui/menus/menu_chat", "chat", _("Chat"), &chat_, true);
		chat_.open_window = [this] {
			if (chat_provider_) {
				GameChatMenu::create_chat_console(this, chat_, *chat_provider_);
			}
		};
		toolbar()->add_space(15);
	}

	add_toolbar_button(
	   "wui/menus/menu_objectives", "objectives", _("Objectives"), &objectives_, true);
	objectives_.open_window = [this] { new GameObjectivesMenu(this, objectives_); };

	toggle_message_menu_ = add_toolbar_button(
	   "wui/menus/menu_toggle_oldmessage_menu", "messages", _("Messages"), &message_menu_, true);
	message_menu_.open_window = [this] { new GameMessageMenu(*this, message_menu_); };

	add_toolbar_button("ui_basic/menu_help", "help", _("Tribal Encyclopedia"), &encyclopedia_, true);
	encyclopedia_.open_window = [this] {
		new TribalEncyclopedia(*this, encyclopedia_, &game().lua());
	};

	set_player_number(plyn);
	map_view()->field_clicked.connect([this](const Widelands::NodeAndTriangle<>& node_and_triangle) {
			node_action(node_and_triangle);
	});

	adjust_toolbar_position();

	main_windows_.stock.open_window = [this] { new StockMenu(*this, main_windows_.stock); };

#ifndef NDEBUG  //  only in debug builds
	addCommand("switchplayer", boost::bind(&InteractivePlayer::cmdSwitchPlayer, this, _1));
#endif
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
					fieldaction_.destroy();
					map_view()->mouse_to_field(flag_to_connect_, MapView::Transition::Jump);
					set_sel_pos(Widelands::NodeAndTriangle<>(
					   flag_to_connect_,
					   Widelands::TCoords<>(flag_to_connect_, Widelands::TCoords<>::D)));
					start_build_road(flag_to_connect_, field.get_owned_by());
				}
			flag_to_connect_ = Widelands::Coords::null();
		}
	}
	if (is_multiplayer()) {
		toggle_chat_->set_visible(chat_provider_);
		toggle_chat_->set_enabled(chat_provider_);
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
		toggle_message_menu_->set_pic(g_gr->images().get(msg_icon));
		toggle_message_menu_->set_tooltip(msg_tooltip);
	}
}

void InteractivePlayer::draw(RenderTarget& dst) {
	// Bail out if the game isn't actually loaded.
	// This fixes a crash with displaying an error dialog during loading.
	if (!game().is_loaded())
		return;

	draw_map_view(map_view(), &dst);
}

void InteractivePlayer::draw_map_view(MapView* map_view, RenderTarget* dst) {
	const Widelands::Player& plr = player();
	const auto& gbase = egbase();
	const Widelands::Map& map = gbase.map();
	const uint32_t gametime = gbase.get_gametime();

	auto* fields_to_draw = map_view->draw_terrain(gbase, dst);
	const auto roads_preview = road_building_preview();

	for (size_t idx = 0; idx < fields_to_draw->size(); ++idx) {
		auto* f = fields_to_draw->mutable_field(idx);

		// Adjust this field for visibility for this player.
		if (!plr.see_all()) {
			const Widelands::Player::Field& pf =
			   plr.fields()[map.get_index(f->fcoords, map.get_width())];

			f->brightness = adjusted_field_brightness(f->fcoords, gametime, pf);
			f->roads = pf.roads;
			f->vision = pf.vision;
			if (pf.vision == 1) {
				f->owner = pf.owner != 0 ? &gbase.player(pf.owner) : nullptr;
				f->is_border = pf.border;
			}
		}

		// Add road building overlays.
		const auto it = roads_preview.find(f->fcoords);
		if (it != roads_preview.end()) {
			f->roads |= it->second;
		}
	}

	draw_objects(
	   gbase, 1.f / map_view->view().zoom, *fields_to_draw, plr, get_text_to_draw(), dst);
}

void InteractivePlayer::popup_message(Widelands::MessageId const id,
                                      const Widelands::Message& message) {
	message_menu_.create();
	dynamic_cast<GameMessageMenu&>(*message_menu_.window).show_new_message(id, message);
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

int32_t InteractivePlayer::calculate_buildcaps(const Widelands::FCoords& c) {
	assert(get_player());
	return get_player()->get_buildcaps(c);
}

/// Player has clicked on the given node; bring up the context menu.
void InteractivePlayer::node_action(const Widelands::NodeAndTriangle<>& node_and_triangle) {
	const Map& map = egbase().map();
	if (1 < player().vision(Map::get_index(node_and_triangle.node, map.get_width()))) {
		// Special case for buildings
		if (upcast(Building, building, map.get_immovable(node_and_triangle.node)))
			if (can_see(building->owner().player_number())) {
				show_building_window(node_and_triangle.node, false);
				return;
			}

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
			minimap_registry().toggle();
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
			FALLS_THROUGH;
		case SDLK_HOME:
			map_view()->scroll_to_field(
			   game().map().get_starting_pos(player_number_), MapView::Transition::Smooth);
			return true;

		case SDLK_KP_ENTER:
		case SDLK_RETURN:
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
	if (n < 1 || n > kMaxPlayers || !game().get_player(n)) {
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
