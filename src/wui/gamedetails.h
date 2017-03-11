/*
 * Copyright (C) 2016 by the Widelands Development Team
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
#include "logic/game_controller.h"
#include "ui_basic/box.h"
#include "ui_basic/icon.h"
#include "ui_basic/multilinetextarea.h"

/**
 * Data about a savegame/replay that we're interested in.
 */
struct SavegameData {
	std::string filename;
	std::string filename_list;
	std::string mapname;
	std::string wincondition;
	std::string minimap_path;
	std::string savedatestring;
	std::string errormessage;

	std::string gametime;
	std::string nrplayers;
	std::string version;
	time_t savetimestamp;
	GameController::GameType gametype;

	SavegameData();

	void set_gametime(uint32_t input_gametime);
	void set_nrplayers(Widelands::PlayerNumber input_nrplayers);
	void set_mapname(const std::string& input_mapname);
};

/**
 * Show a Panel with information about a map.
 */
class GameDetails : public UI::Box {
public:
	enum class Style { kFsMenu, kWui };

	GameDetails(Panel* parent, Style style);

	void clear();
	void update(const SavegameData& gamedata);
	UI::Box* button_box() {
		return button_box_;
	}

private:
	void layout() override;

	const Style style_;
	const int padding_;

	UI::MultilineTextarea name_label_;
	UI::MultilineTextarea descr_;
	UI::Icon minimap_icon_;
	std::unique_ptr<const Image> minimap_image_;
	UI::Box* button_box_;
};

#endif  // end of include guard: WL_WUI_GAMEDETAILS_H
