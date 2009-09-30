/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef GENERAL_STATISTICS_MENU_H
#define GENERAL_STATISTICS_MENU_H

#include "constants.h"

#include "plot_area.h"

#include "ui_basic/radiobutton.h"
#include "ui_basic/unique_window.h"

struct Interactive_GameBase;
namespace UI {
struct Checkbox;
struct Radiogroup;
}

struct General_Statistics_Menu : public UI::UniqueWindow {
	General_Statistics_Menu
		(Interactive_GameBase &, UI::UniqueWindow::Registry &);

private:
	WUIPlot_Area         m_plot;
	UI::Radiogroup       m_radiogroup;
	int32_t              m_selected_information;
	UI::Checkbox       * m_cbs[MAX_PLAYERS];

	void clicked_help();
	void cb_changed_to(int32_t, bool);
	void radiogroup_changed(int32_t);
};

#endif
