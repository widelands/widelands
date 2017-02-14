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

#include "wui/militarysitewindow.h"

#include "graphic/graphic.h"
#include "ui_basic/tabpanel.h"
#include "wui/soldiercapacitycontrol.h"
#include "wui/soldierlist.h"

using namespace Widelands;

static char const* pic_tab_military = "images/wui/buildings/menu_tab_military.png";

MilitarySiteWindow::MilitarySiteWindow(InteractiveGameBase& parent,
                                       UI::UniqueWindow::Registry& reg,
                                       MilitarySite& ms,
                                       bool avoid_fastclick)
   : BuildingWindow(parent, reg, ms, avoid_fastclick) {
	init(avoid_fastclick);
}

void MilitarySiteWindow::create_capsbuttons(UI::Box* buttons) {
	BuildingWindow::create_capsbuttons(buttons);
}

void MilitarySiteWindow::init(bool avoid_fastclick) {
	BuildingWindow::init(avoid_fastclick);
	get_tabs()->add("soldiers", g_gr->images().get(pic_tab_military),
	                create_soldier_list(*get_tabs(), igbase(), militarysite()), _("Soldiers"));
	think();
}
