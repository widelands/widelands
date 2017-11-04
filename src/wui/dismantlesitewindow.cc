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

#include "wui/dismantlesitewindow.h"

#include "graphic/graphic.h"
#include "ui_basic/tabpanel.h"

static const char pic_tab_wares[] = "images/wui/buildings/menu_tab_wares.png";

DismantleSiteWindow::DismantleSiteWindow(InteractiveGameBase& parent,
                                         UI::UniqueWindow::Registry& reg,
                                         Widelands::DismantleSite& ds,
                                         bool avoid_fastclick)
   : BuildingWindow(parent, reg, ds, avoid_fastclick), progress_(nullptr) {
	init(avoid_fastclick);
}

void DismantleSiteWindow::init(bool avoid_fastclick) {
	BuildingWindow::init(avoid_fastclick);
	Widelands::DismantleSite& ds = dynamic_cast<Widelands::DismantleSite&>(building());
	UI::Box& box = *new UI::Box(get_tabs(), 0, 0, UI::Box::Vertical);

	// Add the progress bar
	progress_ = new UI::ProgressBar(&box, 0, 0, UI::ProgressBar::DefaultWidth,
	                                UI::ProgressBar::DefaultHeight, UI::ProgressBar::Horizontal);
	progress_->set_total(1 << 16);
	box.add(progress_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	box.add_space(8);

	// Add the wares queue
	for (uint32_t i = 0; i < ds.get_nrwaresqueues(); ++i)
		BuildingWindow::create_input_queue_panel(&box, ds, ds.get_waresqueue(i), true);

	get_tabs()->add("wares", g_gr->images().get(pic_tab_wares), &box, _("Building materials"));
	think();
}

/*
===============
Make sure the window is redrawn when necessary.
===============
*/
void DismantleSiteWindow::think() {
	BuildingWindow::think();

	const Widelands::DismantleSite& ds = dynamic_cast<Widelands::DismantleSite&>(building());

	progress_->set_state(ds.get_built_per64k());
}
