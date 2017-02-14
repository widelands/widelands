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

#include "wui/trainingsitewindow.h"

#include "graphic/graphic.h"
#include "ui_basic/tabpanel.h"
#include "wui/soldiercapacitycontrol.h"
#include "wui/soldierlist.h"

static char const* pic_tab_military = "images/wui/buildings/menu_tab_military.png";

using namespace Widelands;

/**
 * Create the \ref TrainingSite specific soldier list tab.
 */
TrainingSiteWindow::TrainingSiteWindow(InteractiveGameBase& parent,
                                       UI::UniqueWindow::Registry& reg,
                                       TrainingSite& ts,
                                       bool avoid_fastclick)
   : ProductionSiteWindow(parent, reg, ts, avoid_fastclick) {
	init(avoid_fastclick);
}

void TrainingSiteWindow::init(bool avoid_fastclick) {
	ProductionSiteWindow::init(avoid_fastclick);
	get_tabs()->add("soldiers", g_gr->images().get(pic_tab_military),
	                create_soldier_list(*get_tabs(), igbase(), trainingsite()),
	                _("Soldiers in training"));
	think();
}

void TrainingSiteWindow::create_capsbuttons(UI::Box* buttons) {
	ProductionSiteWindow::create_capsbuttons(buttons);
}
