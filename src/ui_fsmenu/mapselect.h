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

#ifndef WL_UI_FSMENU_MAPSELECT_H
#define WL_UI_FSMENU_MAPSELECT_H

#include <memory>

#include "logic/game.h"
#include "ui_basic/box.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_fsmenu/menu.h"
#include "wui/mapdetails.h"
#include "wui/maptable.h"

using Widelands::Map;
struct GameSettingsProvider;

namespace FsMenu {

class LaunchMPG;

/**
 * Select a Map in Fullscreen Mode. It's a modal fullscreen menu
 */
class MapSelect : public TwoColumnsFullNavigationMenu {
public:
	MapSelect(MenuCapsule&,
	          LaunchMPG* /* nullptr for single player */,
	          GameSettingsProvider*,
	          GameController*,
	          std::shared_ptr<Widelands::Game> for_preview);
	~MapSelect() override;

	MapData const* get_map() const;
	void think() override;

protected:
	void clicked_ok() override;
	void clicked_back() override;
	void entry_selected();
	void fill_table();

private:
	bool compare_players(uint32_t, uint32_t);
	bool compare_mapnames(uint32_t, uint32_t);
	bool compare_size(uint32_t, uint32_t);

	LaunchMPG* parent_screen_;

	/// Updates buttons and text labels and returns whether a table entry is selected.
	bool set_has_selection();
	UI::Checkbox* add_tag_checkbox(UI::Box* box, const std::string& tag, const std::string&);
	void tagbox_changed(int32_t, bool);
	void clear_filter();
	void rebuild_balancing_dropdown();

	std::shared_ptr<Widelands::Game> game_for_preview_;

	UI::Box checkboxes_;

	MapTable table_;
	MapDetails map_details_;

	Map::ScenarioTypes scenario_types_;

	const std::string basedir_;
	std::string curdir_;

	GameSettingsProvider* settings_;
	GameController* ctrl_;

	UI::Checkbox* cb_dont_localize_mapnames_;
	bool has_translated_mapname_;

	UI::Button* show_all_maps_;
	std::vector<UI::Checkbox*> tags_checkboxes_;

	UI::Dropdown<std::string>* official_tags_dropdown_;

	UI::Dropdown<std::string>* balancing_tags_dropdown_;
	bool unspecified_balancing_found_;  // Backwards compatibility

	UI::Dropdown<std::string>* team_tags_dropdown_;

	std::vector<std::string> tags_ordered_;
	std::set<uint32_t> req_tags_;

	std::vector<MapData> maps_data_;

	bool update_map_details_;
};
}  // namespace FsMenu
#endif  // end of include guard: WL_UI_FSMENU_MAPSELECT_H
