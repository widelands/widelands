/*
 * Copyright (C) 2016-2019 by the Widelands Development Team
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

#ifndef WL_WUI_GAMEDETAILS_H
#define WL_WUI_GAMEDETAILS_H

#include <memory>

#include "graphic/image.h"
#include "io/filesystem/filesystem.h"
#include "logic/game_controller.h"
#include "logic/widelands.h"
#include "ui_basic/box.h"
#include "ui_basic/icon.h"
#include "ui_basic/multilinetextarea.h"

/**
 * Data about a savegame/replay that we're interested in.
 */
struct SavegameData {
	enum class SavegameType { kSavegame, kParentDirectory, kSubDirectory };
	/// The filename of the currenty selected file
	std::string filename;
	/// List of filenames when multiple files have been selected
	std::string filename_list;
	/// The name of the map that the game is based on
	std::string mapname;
	/// The win condition that was played
	std::string wincondition;
	/// Filename of the minimap or empty if none available
	std::string minimap_path;
	/// "saved on ..."
	std::string savedatestring;
	/// Verbose date and time
	std::string savedonstring;
	/// An error message or empty if no error occurred
	std::string errormessage;

	/// Compact gametime information
	std::string gametime;
	/// Number of players on the map
	std::string nrplayers;
	/// The version of Widelands that the game was played with
	std::string version;
	/// Gametime as time stamp. For games, it's the time the game ended. For replays, it's the time
	/// the game started.
	time_t savetimestamp;
	/// Single payer, nethost, netclient or replay
	GameController::GameType gametype;

	SavegameData();
	SavegameData(const std::string& filename);
	SavegameData(const std::string& filename, const SavegameType& type);
	static SavegameData create_parent_dir(const std::string& current_dir);
	static SavegameData create_sub_dir(const std::string& directory);

	/// Converts timestamp to UI string and assigns it to gametime
	void set_gametime(uint32_t input_gametime);
	/// Sets the number of players on the map as a string
	void set_nrplayers(Widelands::PlayerNumber input_nrplayers);
	/// Sets the mapname as a localized string
	void set_mapname(const std::string& input_mapname);

	bool is_directory() const;

	bool is_parent_directory() const;

	bool is_sub_directory() const;

	bool compare_save_time(const SavegameData& other) const;

	bool compare_directories(const SavegameData& other) const;

	bool compare_map_name(const SavegameData& other) const;

private:
	/// Savegame or directory
	SavegameType type_;
};

/**
 * Show a Panel with information about a savegame/replay file
 */
class GameDetails : public UI::Box {
public:
	enum class Mode { kSavegame, kReplay };

	GameDetails(Panel* parent, UI::PanelStyle style, Mode mode);

	/// Reset the data
	void clear();

	/// Update the display from the 'gamedata'
	void update(const SavegameData& gamedata);

	/// Box on the bottom where extra buttons can be placed from the outside, e.g. a delete button.
	UI::Box* button_box() {
		return button_box_;
	}

private:
	/// Layout the information on screen
	void layout() override;

	const UI::PanelStyle style_;
	const Mode mode_;
	const int padding_;

	UI::MultilineTextarea name_label_;
	UI::MultilineTextarea descr_;
	UI::Icon minimap_icon_;
	std::unique_ptr<const Image> minimap_image_;
	UI::Box* button_box_;
};

#endif  // end of include guard: WL_WUI_GAMEDETAILS_H
