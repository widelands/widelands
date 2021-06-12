/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include <memory>

#include "io/profile.h"
#include "logic/game.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/unique_window.h"
#include "wui/general_statistics_menu.h"
#include "wui/interactive_base.h"

class InteractiveGameBase : public InteractiveBase {
public:
	InteractiveGameBase(Widelands::Game&,
	                    Section& global_s,
	                    bool multiplayer,
	                    ChatProvider* chat_provider);
	~InteractiveGameBase() override {
	}
	Widelands::Game* get_game() const override;
	Widelands::Game& game() const override;

	// Only the 'InteractiveGameBase' has all information of what should be
	// drawn into a map_view (i.e. which overlays are available). The
	// 'WatchWindow' does not have this information, but needs to draw
	// 'map_views', hence this function.
	virtual void draw_map_view(MapView* given_map_view, RenderTarget* dst) = 0;

	void set_sel_pos(Widelands::NodeAndTriangle<> const center) override;

	virtual void node_action(const Widelands::NodeAndTriangle<>& node_and_triangle) = 0;

	void show_watch_window(Widelands::Bob&);

	bool try_show_ship_window();
	bool is_multiplayer() {
		return multiplayer_;
	}

	void show_game_summary();
	/// For the game host. Show a window and ask the host player what to do with the tribe of the
	/// leaving client.
	bool show_game_client_disconnected();
	void postload() override;
	void start() override;
	void toggle_mainmenu();
	void rebuild_main_menu();

protected:
	// For referencing the items in showhidemenu_
	enum class ShowHideEntry {
		kBuildingSpaces,
		kCensus,
		kStatistics,
		kSoldierLevels,
		kWorkareaOverlap,
		kBuildings
	};

	// Adds the mapviewmenu_ to the toolbar
	void add_main_menu();
	// Adds the showhidemenu_ to the toolbar
	void add_showhide_menu();
	void rebuild_showhide_menu() override;
	// Adds the gamespeedmenu_ to the toolbar
	void add_gamespeed_menu();

	// Adds a chat toolbar button and registers the chat console window
	void add_chat_ui();

	bool handle_key(bool down, SDL_Keysym code) override;

	void draw_overlay(RenderTarget&) override;

public:
	// All unique menu windows
	struct GameMenuWindows {
		UI::UniqueWindow::Registry sound_options;
		UI::UniqueWindow::Registry savegame;
		UI::UniqueWindow::Registry loadgame;

		GeneralStatisticsMenu::Registry stats_general;
		UI::UniqueWindow::Registry stats_wares;
		UI::UniqueWindow::Registry stats_stock;
		UI::UniqueWindow::Registry stats_buildings;
		UI::UniqueWindow::Registry stats_soldiers;
		UI::UniqueWindow::Registry stats_seafaring;

		UI::UniqueWindow::Registry help;
	} menu_windows_;

protected:
	UI::UniqueWindow::Registry chat_;
	bool multiplayer_;

	// Show / Hide menu on the toolbar
	UI::Dropdown<ShowHideEntry> showhidemenu_;

	UI::UniqueWindow::Registry fieldaction_;
	UI::UniqueWindow::Registry game_summary_;
	UI::UniqueWindow::Registry client_disconnected_;

private:
	// For referencing the items in mainmenu_
	enum class MainMenuEntry {
#ifndef NDEBUG  //  only in debug builds
		kScriptConsole,
#endif
		kOptions,
		kSaveMap,
		kLoadMap,
		kRestartScenario,
		kExitGame
	};

	// For referencing the items in gamespeedmenu_
	enum class GameSpeedEntry { kIncrease, kDecrease, kPause };

	// Takes the appropriate action when an item in the mainmenu_ is selected
	void main_menu_selected(MainMenuEntry entry);
	// Takes the appropriate action when an item in the showhidemenu_ is selected
	void showhide_menu_selected(ShowHideEntry entry);
	// Takes the appropriate action when an item in the gamespeedmenu_ is selected
	void gamespeed_menu_selected(GameSpeedEntry entry);
	// Rebuilds the gamespeedmenu_ according to current game settings
	void rebuild_gamespeed_menu();

	// Increases the gamespeed
	void increase_gamespeed(uint16_t speed);
	// Decreases the gamespeed
	void decrease_gamespeed(uint16_t speed);
	// Pauses / Unpauses the game and calls rebuild_gamespeed_menu
	void toggle_game_paused();
	// Resets the speed to 1x
	void reset_gamespeed();

	// Main menu on the toolbar
	UI::Dropdown<MainMenuEntry> mainmenu_;
	// Game speed menu on the toolbar
	UI::Dropdown<GameSpeedEntry> gamespeedmenu_;
};

#endif  // end of include guard: WL_WUI_INTERACTIVE_GAMEBASE_H
