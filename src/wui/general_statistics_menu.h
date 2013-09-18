/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#ifndef GENERAL_STATISTICS_MENU_H
#define GENERAL_STATISTICS_MENU_H

#include "constants.h"
#include "wui/plot_area.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/unique_window.h"

class Interactive_GameBase;
namespace UI {
struct Radiogroup;
}

struct General_Statistics_Menu : public UI::UniqueWindow {

	// Custom registry, to store the selected_information as well.
	struct Registry : public UI::UniqueWindow::Registry {
		Registry() :
			UI::UniqueWindow::Registry(),
			selected_information(0),
			selected_players(true, MAX_PLAYERS),
			time(WUIPlot_Area::TIME_GAME)
		{}

		int32_t selected_information;
		std::vector<bool> selected_players;
		WUIPlot_Area::TIME time;
	};

	General_Statistics_Menu
		(Interactive_GameBase &, Registry &);
	virtual ~General_Statistics_Menu();

private:
	Registry           * m_my_registry;
	UI::Box              m_box;
	WUIPlot_Area         m_plot;
	UI::Radiogroup       m_radiogroup;
	int32_t              m_selected_information;
	UI::Button         * m_cbs[MAX_PLAYERS];
	uint32_t             m_ndatasets;

	void clicked_help();
	void cb_changed_to(int32_t);
	void radiogroup_changed(int32_t);
};

#endif
