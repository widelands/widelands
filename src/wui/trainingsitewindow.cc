/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include "logic/map_objects/tribes/trainingsite.h"

#include "graphic/graphic.h"
#include "ui_basic/tabpanel.h"
#include "wui/productionsitewindow.h"
#include "wui/soldiercapacitycontrol.h"
#include "wui/soldierlist.h"

using Widelands::TrainingSite;

static char const* pic_tab_military = "images/wui/buildings/menu_tab_military.png";

/**
 * Status window for \ref TrainingSite
 */
struct TrainingSiteWindow : public ProductionSiteWindow {
	TrainingSiteWindow(InteractiveGameBase& parent, TrainingSite&, UI::Window*& registry);

	TrainingSite& trainingsite() {
		return dynamic_cast<TrainingSite&>(building());
	}

protected:
	void create_capsbuttons(UI::Box* buttons) override;
};

/**
 * Create the \ref TrainingSite specific soldier list tab.
 */
TrainingSiteWindow::TrainingSiteWindow(InteractiveGameBase& parent,
                                       TrainingSite& ts,
                                       UI::Window*& registry)
   : ProductionSiteWindow(parent, ts, registry) {
	get_tabs()->add("soldiers", g_gr->images().get(pic_tab_military),
	                create_soldier_list(*get_tabs(), parent, trainingsite()),
	                _("Soldiers in training"));
}

void TrainingSiteWindow::create_capsbuttons(UI::Box* buttons) {
	ProductionSiteWindow::create_capsbuttons(buttons);
}

/*
===============
Create the training site information window.
===============
*/
void TrainingSite::create_options_window(InteractiveGameBase& plr, UI::Window*& registry) {
	ProductionSiteWindow* win
		= dynamic_cast<ProductionSiteWindow*>(new TrainingSiteWindow(plr, *this, registry));
	Building::options_window_connections.push_back(Building::workers_changed.connect(
	   boost::bind(&ProductionSiteWindow::update_worker_table, boost::ref(*win))));
}
