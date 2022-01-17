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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_UI_FSMENU_SCENARIO_SELECT_H
#define WL_UI_FSMENU_SCENARIO_SELECT_H

#include "ui_basic/dropdown.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/menu.h"
#include "ui_fsmenu/scenariodetails.h"
namespace FsMenu {
/*
 * Fullscreen Menu for selecting a campaign or tutorial scenario
 */
class ScenarioSelect : public TwoColumnsFullNavigationMenu {
public:
	// If camp is not set, we'll be loading the tutorials
	explicit ScenarioSelect(MenuCapsule&, CampaignData* camp);

protected:
	void clicked_ok() override;
	void entry_selected();
	void fill_table();

private:
	void layout() override;

	/// Updates buttons and text labels and returns whether a table entry is selected.
	bool set_has_selection();

	bool is_tutorial_;
	UI::Table<uintptr_t const> table_;

	UI::MultilineTextarea subtitle_;
	ScenarioDetails scenario_details_;
	UI::Textarea scenario_difficulty_header_;
	UI::Dropdown<uint32_t> scenario_difficulty_;

	CampaignData* campaign_;

	std::vector<ScenarioData> scenarios_data_;
};
}  // namespace FsMenu
#endif  // end of include guard: WL_UI_FSMENU_SCENARIO_SELECT_H
