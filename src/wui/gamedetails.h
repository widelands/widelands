/*
 * Copyright (C) 2016-2022 by the Widelands Development Team
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

#ifndef WL_WUI_GAMEDETAILS_H
#define WL_WUI_GAMEDETAILS_H

#include <memory>

#include "graphic/texture.h"
#include "logic/editor_game_base.h"
#include "logic/game_controller.h"
#include "ui_basic/box.h"
#include "ui_basic/icon.h"
#include "ui_basic/multilinetextarea.h"
#include "wui/savegamedata.h"

/**
 * Show a Panel with information about a savegame/replay file
 */
class GameDetails : public UI::Panel {
public:
	enum class Mode { kSavegame, kReplay };

	GameDetails(Panel* parent, UI::PanelStyle style, Mode mode, Widelands::EditorGameBase& egbase);

	/// Reset the data
	void clear();

	/// show details of savegames including minimap
	void display(const std::vector<SavegameData>& gamedata);

	/// Box on the bottom where extra buttons can be placed from the outside, e.g. a delete button.
	UI::Box* button_box() {
		return button_box_;
	}

	bool has_conflicts() const {
		return has_conflicts_;
	}

private:
	/// Layout the information on screen
	void layout() override;
	/// Update the display from the 'gamedata'
	void show(const SavegameData& gamedata);
	void show(const std::vector<SavegameData>& gamedata);
	void show_game_description(const SavegameData& gamedata);
	std::string show_minimap(const SavegameData& gamedata);

	const Mode mode_;
	const int padding_;
	bool has_conflicts_;

	UI::Box main_box_, descr_box_;
	UI::MultilineTextarea name_label_;
	UI::MultilineTextarea descr_;
	UI::Icon minimap_icon_;
	UI::Box* button_box_;

	// Used to render map preview
	std::string last_game_;
	std::unordered_map<std::string, std::unique_ptr<const Texture>> minimap_cache_;
	Widelands::EditorGameBase& egbase_;  // Not owned
};

#endif  // end of include guard: WL_WUI_GAMEDETAILS_H
