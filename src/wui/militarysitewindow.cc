/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "logic/map_objects/tribes/militarysite.h"

#include "graphic/graphic.h"
#include "ui_basic/tabpanel.h"
#include "wui/buildingwindow.h"
#include "wui/soldiercapacitycontrol.h"
#include "wui/soldierlist.h"

using Widelands::MilitarySite;

static char const* pic_tab_military = "images/wui/buildings/menu_tab_military.png";

/**
 * Status window for \ref MilitarySite
 */
struct MilitarySiteWindow : public BuildingWindow {
	MilitarySiteWindow(InteractiveGameBase& parent, MilitarySite&, UI::Window*& registry);

	MilitarySite& militarysite() {
		return dynamic_cast<MilitarySite&>(building());
	}

protected:
	void create_capsbuttons(UI::Box* buttons) override;
};

MilitarySiteWindow::MilitarySiteWindow(InteractiveGameBase& parent,
                                       MilitarySite& ms,
                                       UI::Window*& registry)
   : BuildingWindow(parent, ms, registry) {
	get_tabs()->add("soldiers", g_gr->images().get(pic_tab_military),
	                create_soldier_list(*get_tabs(), parent, militarysite()), _("Soldiers"));
}

void MilitarySiteWindow::create_capsbuttons(UI::Box* buttons) {
	BuildingWindow::create_capsbuttons(buttons);
}

/**
 * Create the  military site information window.
 */
void MilitarySite::create_options_window(InteractiveGameBase& plr, UI::Window*& registry) {
	new MilitarySiteWindow(plr, *this, registry);
}
