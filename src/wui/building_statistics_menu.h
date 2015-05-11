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
#include "ui_basic/tabpanel.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/unique_window.h"

using namespace Widelands;

class InteractivePlayer;

struct BuildingStatisticsMenu : public UI::UniqueWindow {
	BuildingStatisticsMenu
		(InteractivePlayer &, UI::UniqueWindow::Registry &);

	void think() override;
	void update();
	bool handle_key(bool const down, SDL_Keysym const code) override;

private:
	enum class JumpTarget {
		Owned,
		Construction,
		Unproductive
	};

	void add_button(BuildingIndex id, const BuildingDescr& descr, UI::Box& tab);
	void jump_building(BuildingIndex id, JumpTarget target);
	int32_t validate_pointer(int32_t *, int32_t);

	InteractivePlayer& iplayer() const;

	bool is_shift_pressed_;
	UI::MultilineTextarea helptext_;

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
	std::vector<UI::Button*> building_buttons_;
	std::vector<UI::Button*> owned_buttons_;
	std::vector<UI::Button*> productivity_buttons_;
	int32_t last_building_index_;
	BuildingIndex last_building_type_;
	uint32_t lastupdate_;
};

#endif  // end of include guard: WL_WUI_BUILDING_STATISTICS_MENU_H
