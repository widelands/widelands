/*
 * Copyright (C) 2002-2004, 2006, 2008-2009, 2011 by the Widelands Development Team
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

#ifndef WL_WUI_BUILDING_STATISTICS_MENU_H
#define WL_WUI_BUILDING_STATISTICS_MENU_H

#include "logic/building.h"
#include "logic/widelands.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/progressbar.h"
#include "ui_basic/table.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

using namespace Widelands;

class InteractivePlayer;

struct BuildingStatisticsMenu : public UI::UniqueWindow {
	BuildingStatisticsMenu
		(InteractivePlayer &, UI::UniqueWindow::Registry &);

	void think() override;
	void update();

private:
	void init();
	void add_button(BuildingIndex id, const BuildingDescr& descr, UI::Box& tab);

	bool compare_building_size(uint32_t rowa, uint32_t rowb);

	InteractivePlayer & iplayer() const;
	enum JumpTargets {
		PrevOwned,        NextOwned,
		PrevConstruction, NextConstruction,
		PrevUnproductive, NextUnproductive
	};

	UI::TabPanel tabs_;
	UI::Box small_tab_;
	UI::Box medium_tab_;
	UI::Box big_tab_;
	UI::Box mines_tab_;
	UI::Box ports_tab_;

	std::vector<BuildingIndex> small_buildings_;
	std::vector<BuildingIndex> medium_buildings_;
	std::vector<BuildingIndex> big_buildings_;
	std::vector<BuildingIndex> mines_;
	std::vector<BuildingIndex> ports_;
	std::vector<UI::Button*> buttons_;

	// Old table
	UI::Box old_design_;
	UI::Table<uintptr_t const> m_table;
	UI::ProgressBar          m_progbar;
	UI::Textarea              m_total_productivity_label;
	UI::Textarea*              m_owned_label;
	UI::Textarea*              m_owned;
	UI::Textarea*              m_in_build_label;
	UI::Textarea*              m_in_build;
	UI::Textarea*              m_unproductive_label;
	uint32_t                  m_lastupdate;
	uint32_t                  m_end_of_table_y;
	UI::Button * m_btn[6];
	int32_t                   m_last_building_index;
	uint32_t                  m_last_table_index;

	void clicked_help();
	void clicked_jump(JumpTargets);
	void table_changed(uint32_t);
	int32_t validate_pointer(int32_t *, int32_t);
};

#endif  // end of include guard: WL_WUI_BUILDING_STATISTICS_MENU_H
