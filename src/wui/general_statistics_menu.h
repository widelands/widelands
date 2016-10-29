/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#ifndef WL_WUI_GENERAL_STATISTICS_MENU_H
#define WL_WUI_GENERAL_STATISTICS_MENU_H

#include "graphic/playercolor.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/unique_window.h"
#include "wui/plot_area.h"

class InteractiveGameBase;
namespace UI {
struct Radiogroup;
}

struct GeneralStatisticsMenu : public UI::UniqueWindow {

	// Custom registry, to store the selected_information as well.
	struct Registry : public UI::UniqueWindow::Registry {
		Registry()
		   : UI::UniqueWindow::Registry(),
		     selected_information(0),
		     selected_players(true, kMaxPlayers),
		     time(WuiPlotArea::TIME_GAME) {
		}

		int32_t selected_information;
		std::vector<bool> selected_players;
		WuiPlotArea::TIME time;
	};

	GeneralStatisticsMenu(InteractiveGameBase&, Registry&);
	virtual ~GeneralStatisticsMenu();

private:
	Registry* my_registry_;
	UI::Box box_;
	WuiPlotArea plot_;
	UI::Radiogroup radiogroup_;
	int32_t selected_information_;
	UI::Button* cbs_[kMaxPlayers];
	uint32_t ndatasets_;

	void clicked_help();
	void cb_changed_to(int32_t);
	void radiogroup_changed(int32_t);
};

#endif  // end of include guard: WL_WUI_GENERAL_STATISTICS_MENU_H
