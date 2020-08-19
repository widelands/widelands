/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#ifndef WL_WUI_INTERACTIVE_PLAYER_H
#define WL_WUI_INTERACTIVE_PLAYER_H

#include <memory>

#include "io/profile.h"
#include "logic/message_id.h"
#include "logic/note_map_options.h"
#include "ui_basic/button.h"
#include "wui/interactive_gamebase.h"

/**
 * This is the interactive player. this one is
 * responsible to show the correct map
 * to the player and draws the user interface,
 * cares for input and so on.
 */
class InteractivePlayer : public InteractiveGameBase {
public:
	InteractivePlayer(Widelands::Game&,
	                  Section& global_s,
	                  Widelands::PlayerNumber,
	                  bool multiplayer,
	                  ChatProvider* chat_provider = nullptr);

	bool can_see(Widelands::PlayerNumber) const override;
	bool can_act(Widelands::PlayerNumber) const override;
	Widelands::PlayerNumber player_number() const override;
	void draw_map_view(MapView* given_map_view, RenderTarget* dst) override;

	void node_action(const Widelands::NodeAndTriangle<>& node_and_triangle) override;

	bool handle_key(bool down, SDL_Keysym) override;

	const Widelands::Player& player() const {
		return game().player(player_number_);
	}
	Widelands::Player* get_player() const override {
		assert(&game());
		return game().get_player(player_number_);
	}

	// for savegames
	void set_player_number(uint32_t plrn);

	// For load
	void cleanup_for_load() override;
	void postload() override;
	void think() override;
	void draw(RenderTarget& dst) override;

	std::map<Widelands::Ship*, Widelands::Coords>& get_expedition_port_spaces() {
		return expedition_port_spaces_;
	}
	bool has_expedition_port_space(const Widelands::Coords&) const;

	void set_flag_to_connect(const Widelands::Coords& location) {
		flag_to_connect_ = location;
	}

	void popup_message(Widelands::MessageId, const Widelands::Message&);

private:
	// For referencing the items in statisticsmenu_
	enum class StatisticsMenuEntry { kGeneral, kWare, kBuildings, kStock, kSoldiers, kSeafaring };

	// Adds the statisticsmenu_ to the toolbar
	void add_statistics_menu();
	// Rebuilds the statisticsmenu_ according to current map settings
	void rebuild_statistics_menu();
	// Takes the appropriate action when an item in the statisticsmenu_ is selected
	void statistics_menu_selected(StatisticsMenuEntry entry);
	void rebuild_showhide_menu() override;

	bool player_hears_field(const Widelands::Coords& coords) const override;

	void cmdSwitchPlayer(const std::vector<std::string>& args);

	Widelands::PlayerNumber player_number_;
	bool auto_roadbuild_mode_;
	Widelands::Coords flag_to_connect_;

	UI::Button* toggle_message_menu_;

	// Statistics menu on the toolbar
	UI::Dropdown<StatisticsMenuEntry> statisticsmenu_;
	UI::UniqueWindow::Registry objectives_;
	UI::UniqueWindow::Registry encyclopedia_;
	UI::UniqueWindow::Registry message_menu_;

	const Image* grid_marker_pic_;

	std::map<Widelands::Ship*, Widelands::Coords> expedition_port_spaces_;

	std::unique_ptr<Notifications::Subscriber<NoteMapOptions>> map_options_subscriber_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteShip>> shipnotes_subscriber_;
};

#endif  // end of include guard: WL_WUI_INTERACTIVE_PLAYER_H
