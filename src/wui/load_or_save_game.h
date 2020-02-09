/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include <memory>

#include "game_io/game_preload_packet.h"
#include "logic/game.h"
#include "ui_basic/box.h"
#include "ui_basic/panel.h"
#include "ui_basic/table.h"
#include "wui/gamedetails.h"
#include "wui/savegametable.h"

/// Common functions for loading or saving a game or replay.
class LoadOrSaveGame {
	friend class FullscreenMenuLoadGame;
	friend struct GameMainMenuSaveGame;

protected:
	/// Choose which type of files to show
	enum class FileType { kShowAll, kGameMultiPlayer, kGameSinglePlayer, kReplay };

	/// A table of savegame/replay files and a game details panel.
	LoadOrSaveGame(UI::Panel* parent,
	               Widelands::Game& g,
	               FileType filetype,
	               UI::PanelStyle style,
	               bool localize_autosave);

	/// Update gamedetails and tooltips and return information about the current selection
	std::unique_ptr<SavegameData> entry_selected();

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

	void change_directory_to(std::string& directory);

private:
	/// Returns the filename for the table entry at 'index'
	const std::string get_filename(uint32_t index) const;
	/// Returns the savegame for the table entry at 'index'
	const SavegameData& get_savegame(uint32_t index) const;
	/// Formats the current table selection as a list of filenames with savedate information.
	const std::string filename_list_string() const;
	/// Formats a given table selection as a list of filenames with savedate information.
	const std::string filename_list_string(const std::set<uint32_t>& selections) const;

	/// Reverse default sort order for save date column
	bool compare_save_time(uint32_t, uint32_t) const;
	bool compare_map_name(uint32_t, uint32_t) const;

	UI::Panel* parent_;
	UI::Box* table_box_;
	FileType filetype_;

	SavegameTable* table_;
	std::vector<SavegameData> games_data_;
	GameDetails game_details_;
	UI::Button* delete_;

	const std::string basedir_;
	std::string curdir_;

	Widelands::Game& game_;
	void load_gamefile(const std::string& gamefilename);
	bool is_valid_gametype(const SavegameData& gamedata) const;
	void add_time_info(SavegameData& gamedata, const Widelands::GamePreloadPacket& gpdp) const;
	void add_general_information(SavegameData& gamedata,
	                             const Widelands::GamePreloadPacket& gpdp) const;
	void add_error_info(SavegameData& gamedata, std::string errormessage) const;
	void add_sub_dir(const std::string& gamefilename);
	bool selection_contains_directory() const;
};

#endif  // end of include guard: WL_WUI_LOAD_OR_SAVE_GAME_H
