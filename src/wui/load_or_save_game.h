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

#ifndef WL_WUI_LOAD_OR_SAVE_GAME_H
#define WL_WUI_LOAD_OR_SAVE_GAME_H

#include "logic/game.h"
#include "ui_basic/box.h"
#include "ui_basic/panel.h"
#include "ui_basic/table.h"
#include "wui/gamedetails.h"

/// Common functions for loading or saving a game or replay.
class LoadOrSaveGame {
	friend class GameMainMenuSaveGame;
	friend class FullscreenMenuLoadGame;

protected:
	/// Choose which type of files to show
	enum class FileType { kReplay, kGame, kGameMultiPlayer, kGameSinglePlayer };

	/// A table of savegame/replay files and a game details panel.
	LoadOrSaveGame(UI::Panel* parent,
	               Widelands::Game& g,
	               FileType filetype,
	               GameDetails::Style style,
	               bool localize_autosave);

	//// Update gamedetails and tooltips and return information about the current selection
	const SavegameData* entry_selected();

	/// Whether the table has a selection
	bool has_selection() const;

	/// Clear table selections and game data
	void clear_selections();

	/// Finds the given filename on the table and selects it
	void select_by_name(const std::string& name);

	/// Read savegame/replay files and fill the table and games data.
	void fill_table();

	/// The table panel
	UI::Table<uintptr_t const>& table();

	/// A vertical box wrapping the table. This can be used to add UI elements above/below the table.
	UI::Box* table_box();

	/// The game details panel
	GameDetails* game_details();

	/// The delete button shown on the bottom of the game details panel
	UI::Button* delete_button();

	/// Show confirmation window and delete the selected file(s)
	void clicked_delete();

private:
	/// Returns the filename for the table entry at 'index'
	const std::string get_filename(int index) const;

	/// Formats the current table selection as a list of filenames with savedate information.
	const std::string filename_list_string() const;

	/// Reverse default sort order for save date column
	bool compare_date_descending(uint32_t, uint32_t) const;

	UI::Panel* parent_;
	UI::Box* table_box_;
	UI::Table<uintptr_t const> table_;
	FileType filetype_;
	bool localize_autosave_;
	std::vector<SavegameData> games_data_;
	GameDetails game_details_;
	UI::Button* delete_;

	Widelands::Game& game_;
};

#endif  // end of include guard: WL_WUI_LOAD_OR_SAVE_GAME_H
