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

#ifndef WL_EDITOR_UI_MENUS_MAIN_MENU_MAP_OPTIONS_H
#define WL_EDITOR_UI_MENUS_MAIN_MENU_MAP_OPTIONS_H

#include "logic/map.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/editbox.h"
#include "ui_basic/icon.h"
#include "ui_basic/multilineeditbox.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

class EditorInteractive;
struct SuggestedTeamsEntry;

/**
 * This is the Main Options Menu. Here, information
 * about the current map are displayed and you can change
 * author, name and description
 */
struct MainMenuMapOptions : public UI::UniqueWindow {
	MainMenuMapOptions(EditorInteractive&, UI::UniqueWindow::Registry& registry);

	void delete_suggested_team(SuggestedTeamsEntry*);

private:
	EditorInteractive& eia();
	void changed();
	void update();
	void clicked_ok();
	void clicked_cancel();
	void add_tag_checkbox(UI::Box* box, const std::string& tag, const std::string& displ_name);

	const unsigned int padding_, indent_, labelh_, checkbox_space_, butw_, max_w_;

	UI::Box tab_box_, buttons_box_;

	UI::Button ok_, cancel_;

	UI::TabPanel tabs_;
	UI::Box main_box_;
	UI::Box tags_box_;
	UI::Box teams_box_, inner_teams_box_;

	UI::EditBox name_, author_;
	UI::Textarea size_;
	UI::MultilineEditbox* descr_;
	UI::MultilineEditbox* hint_;

	// Tag, Checkbox
	std::map<std::string, UI::Checkbox*> tags_checkboxes_;
	UI::Dropdown<std::string> balancing_dropdown_;
	UI::Dropdown<std::string> theme_dropdown_;

	UI::SpinBox* waterway_length_box_;
	UI::Icon* waterway_length_warning_;
	void update_waterway_length_warning();

	std::vector<SuggestedTeamsEntry*> suggested_teams_entries_;
	UI::Button new_suggested_team_;

	UI::UniqueWindow::Registry& registry_;
};

struct SuggestedTeamsEntry : public UI::Panel {
	SuggestedTeamsEntry(MainMenuMapOptions*,
	                    UI::Panel*,
	                    const Widelands::Map&,
	                    unsigned w,
	                    Widelands::SuggestedTeamLineup);
	~SuggestedTeamsEntry() override {
	}

	void layout() override;

	const Widelands::SuggestedTeamLineup& team() const {
		return team_;
	}
	Widelands::SuggestedTeamLineup& team() {
		return team_;
	}

private:
	const Widelands::Map& map_;
	Widelands::SuggestedTeamLineup team_;
	UI::Button delete_;
	std::vector<UI::Dropdown<Widelands::PlayerNumber>*> dropdowns_;
	std::vector<std::vector<UI::Button*>> buttons_;

	UI::Dropdown<Widelands::PlayerNumber>* create_dropdown(size_t);
	UI::Button* create_button(Widelands::PlayerNumber);

	void update();
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_MAIN_MENU_MAP_OPTIONS_H
