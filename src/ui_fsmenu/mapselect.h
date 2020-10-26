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

#ifndef WL_UI_FSMENU_MAPSELECT_H
#define WL_UI_FSMENU_MAPSELECT_H

#include "ui_basic/box.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_fsmenu/load_map_or_game.h"
#include "wui/mapdetails.h"
#include "wui/maptable.h"

using Widelands::Map;
class GameController;
struct GameSettingsProvider;

/**
 * Select a Map in Fullscreen Mode. It's a modal fullscreen menu
 */
class FullscreenMenuMapSelect : public FullscreenMenuLoadMapOrGame {
public:
	FullscreenMenuMapSelect(FullscreenMenuMain&,
	                        GameSettingsProvider*,
	                        GameController*,
	                        Widelands::EditorGameBase& egbase);

	MapData const* get_map() const;
	void think() override;

protected:
	void clicked_ok() override;
	void entry_selected() override;
	void fill_table() override;

private:
	void layout() override;

	bool compare_players(uint32_t, uint32_t);
	bool compare_mapnames(uint32_t, uint32_t);
	bool compare_size(uint32_t, uint32_t);

	/// Updates buttons and text labels and returns whether a table entry is selected.
	bool set_has_selection();
	UI::Checkbox* add_tag_checkbox(UI::Box* box, const std::string& tag, const std::string&);
	void tagbox_changed(int32_t, bool);
	void clear_filter();
	void rebuild_balancing_dropdown();

	int32_t const checkbox_space_;
	const int checkbox_padding_;
	int32_t checkboxes_y_;

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

#endif  // end of include guard: WL_UI_FSMENU_MAPSELECT_H
