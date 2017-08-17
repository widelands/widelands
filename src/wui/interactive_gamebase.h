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

#ifndef WL_WUI_INTERACTIVE_GAMEBASE_H
#define WL_WUI_INTERACTIVE_GAMEBASE_H

#include <map>
#include <memory>

#include "logic/game.h"
#include "profile/profile.h"
#include "ui_basic/unique_window.h"
#include "wui/general_statistics_menu.h"
#include "wui/interactive_base.h"

struct ChatProvider;

enum PlayerType { NONE, OBSERVER, PLAYING, VICTORIOUS, DEFEATED };

class InteractiveGameBase : public InteractiveBase {
public:
	struct GameMainMenuWindows {
		UI::UniqueWindow::Registry loadgame;
		UI::UniqueWindow::Registry savegame;
		UI::UniqueWindow::Registry readme;
		UI::UniqueWindow::Registry keys;
		UI::UniqueWindow::Registry help;
		UI::UniqueWindow::Registry license;
		UI::UniqueWindow::Registry sound_options;

		UI::UniqueWindow::Registry building_stats;
		GeneralStatisticsMenu::Registry general_stats;
		UI::UniqueWindow::Registry ware_stats;
		UI::UniqueWindow::Registry stock;
	};

	InteractiveGameBase(Widelands::Game&,
	                    Section& global_s,
	                    PlayerType pt = NONE,
	                    bool multiplayer = false);
	~InteractiveGameBase() override {
	}
	Widelands::Game* get_game() const;
	Widelands::Game& game() const;

	// Chat messages
	void set_chat_provider(ChatProvider&);
	ChatProvider* get_chat_provider();

	virtual bool can_see(Widelands::PlayerNumber) const = 0;
	virtual bool can_act(Widelands::PlayerNumber) const = 0;
	virtual Widelands::PlayerNumber player_number() const = 0;

	virtual void node_action() = 0;
	const PlayerType& get_playertype() const {
		return playertype_;
	}
	void set_playertype(const PlayerType& pt) {
		playertype_ = pt;
	}

	void add_wanted_building_window(const Widelands::Coords& coords,
	                                const Vector2i point,
	                                bool was_minimal);
	UI::UniqueWindow* show_building_window(const Widelands::Coords& coords, bool avoid_fastclick);
	bool try_show_ship_window();
	bool is_multiplayer() {
		return multiplayer_;
	}

	void show_game_summary();
	void postload() override;
	void start() override {
	}

protected:
	void draw_overlay(RenderTarget&) override;
	virtual int32_t calculate_buildcaps(const Widelands::FCoords& c) = 0;

	GameMainMenuWindows main_windows_;
	ChatProvider* chat_provider_;
	bool multiplayer_;
	PlayerType playertype_;
	UI::UniqueWindow::Registry fieldaction_;
	UI::UniqueWindow::Registry game_summary_;
	UI::Button* toggle_buildhelp_;
	UI::Button* reset_zoom_;

private:
	void on_buildhelp_changed(const bool value) override;
	// Building coordinates, window position, whether the window was minimized
	std::map<uint32_t, std::pair<const Vector2i, bool>> wanted_building_windows_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteBuilding>> buildingnotes_subscriber_;
};

#endif  // end of include guard: WL_WUI_INTERACTIVE_GAMEBASE_H
