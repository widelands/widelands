/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_WUI_LOAD_OR_SAVE_GAME_H
#define WL_WUI_LOAD_OR_SAVE_GAME_H

#include <memory>

#include "base/macros.h"
#include "logic/game.h"
#include "ui_basic/box.h"
#include "ui_basic/panel.h"
#include "ui_basic/table.h"
#include "wui/gamedetails.h"
#include "wui/savegamedeleter.h"
#include "wui/savegameloader.h"
#include "wui/savegametable.h"

/// Common functions for loading or saving a game or replay.
class LoadOrSaveGame {
	friend class LoadGame;
	friend struct GameMainMenuSaveGame;

public:
	/// Choose which type of files to show
	enum class FileType { kShowAll, kGameMultiPlayer, kGameSinglePlayer, kReplay };

	/// A table of savegame/replay files and a game details panel.
	LoadOrSaveGame(UI::Panel* parent,
	               Widelands::Game& g,
	               FileType filetype,
	               UI::PanelStyle,
	               UI::WindowStyle,
	               bool localize_autosave,
	               UI::Panel* table_parent = nullptr,
	               UI::Panel* delete_button_parent = nullptr);

	/// Make cppcheck happy
	DISALLOW_COPY_AND_ASSIGN(LoadOrSaveGame);

	/// Update gamedetails and tooltips and return information about the current selection
	std::unique_ptr<SavegameData> entry_selected();

	bool check_replay_compatibility(const SavegameData& sd);

	/// Whether the table has a selection
	bool has_selection() const;

	/// Clear table selections and game data
	void clear_selections();

	/// Finds the given filename on the table and selects it
	void select_by_name(const std::string& name);

	/// Read savegame/replay files and fill the table and games data.
	void fill_table();

	/// Set whether to show filenames. Has only an effect for Replays.
	void set_show_filenames(bool);

	/// The table panel
	SavegameTable& table();

	/// A vertical box wrapping the table. This can be used to add UI elements above/below the table.
	UI::Box* table_box();

	/// The game details panel
	GameDetails* game_details();

	/// The delete button shown on the bottom of the game details panel
	UI::Button* delete_button();

	/// Show confirmation window and delete the selected file(s)
	void clicked_delete();

	void change_directory_to(const std::string& directory);

private:
	/// Returns the savegame for the table entry at 'index'
	const SavegameData& get_savegame(uint32_t index) const;

	/// Reverse default sort order for save date column
	bool compare_save_time(uint32_t, uint32_t) const;
	bool compare_map_name(uint32_t, uint32_t) const;

	UI::WindowStyle window_style_;
	UI::Box* table_box_;
	FileType filetype_;

	std::unique_ptr<SavegameDeleter> savegame_deleter_;
	std::unique_ptr<SavegameLoader> savegame_loader_;

	SavegameTable* table_;
	std::vector<SavegameData> games_data_;
	GameDetails game_details_;
	UI::Button* delete_;

	const std::string basedir_;
	std::string curdir_;

	Widelands::Game& game_;

	bool selection_contains_directory() const;
	const std::vector<SavegameData> get_selected_savegames() const;
	void set_tooltips_of_buttons(size_t nr_of_selected_items) const;
	void select_item_and_scroll_to_it(std::set<uint32_t>& selections);
};

#endif  // end of include guard: WL_WUI_LOAD_OR_SAVE_GAME_H
