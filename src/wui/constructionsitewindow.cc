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

#include "wui/constructionsitewindow.h"

#include <boost/format.hpp>

#include "graphic/graphic.h"
#include "ui_basic/tabpanel.h"
#include "wui/inputqueuedisplay.h"

static const char pic_tab_wares[] = "images/wui/buildings/menu_tab_wares.png";

ConstructionSiteWindow::ConstructionSiteWindow(InteractiveGameBase& parent,
                                               UI::UniqueWindow::Registry& reg,
                                               Widelands::ConstructionSite& cs,
                                               bool avoid_fastclick)
   : BuildingWindow(parent, reg, cs, avoid_fastclick) {
	init(avoid_fastclick);
}

void ConstructionSiteWindow::init(bool avoid_fastclick) {
	BuildingWindow::init(avoid_fastclick);
	Widelands::ConstructionSite& cs = dynamic_cast<Widelands::ConstructionSite&>(building());
	UI::Box& box = *new UI::Box(get_tabs(), 0, 0, UI::Box::Vertical);

	// Add the progress bar
	progress_ = new UI::ProgressBar(&box, 0, 0, UI::ProgressBar::DefaultWidth,
	                                UI::ProgressBar::DefaultHeight, UI::ProgressBar::Horizontal);
	progress_->set_total(1 << 16);
	box.add(progress_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	box.add_space(8);

	// Add the wares queue
	for (uint32_t i = 0; i < cs.get_nrwaresqueues(); ++i)
		box.add(new InputQueueDisplay(&box, 0, 0, *igbase(), cs, cs.get_waresqueue(i)));

	get_tabs()->add("wares", g_gr->images().get(pic_tab_wares), &box, _("Building materials"));

	set_title((boost::format("(%s)") % cs.building().descname()).str());
	think();
}

/*
===============
Make sure the window is redrawn when necessary.
===============
*/
void ConstructionSiteWindow::think() {
	BuildingWindow::think();

	const Widelands::ConstructionSite& cs = dynamic_cast<Widelands::ConstructionSite&>(building());

	progress_->set_state(cs.get_built_per64k());
}
