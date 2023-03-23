/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wui/dismantlesitewindow.h"

#include "wui/inputqueuedisplay.h"

static const char pic_tab_wares[] = "images/wui/buildings/menu_tab_wares.png";

DismantleSiteWindow::DismantleSiteWindow(InteractiveBase& parent,
                                         BuildingWindow::Registry& reg,
                                         Widelands::DismantleSite& ds,
                                         bool avoid_fastclick)
   : BuildingWindow(parent, reg, ds, avoid_fastclick), dismantle_site_(&ds) {
	init(avoid_fastclick, false);
}

void DismantleSiteWindow::init(bool avoid_fastclick, bool workarea_preview_wanted) {
	Widelands::DismantleSite* dismantle_site = dismantle_site_.get(ibase()->egbase());
	assert(dismantle_site != nullptr);

	BuildingWindow::init(avoid_fastclick, workarea_preview_wanted);
	UI::Box& box = *new UI::Box(get_tabs(), UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);
	UI::Box& subbox = *new UI::Box(&box, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);
	ensure_box_can_hold_input_queues(subbox);

	// Add the progress bar
	progress_ = new UI::ProgressBar(&box, UI::PanelStyle::kWui, 0, 0, UI::ProgressBar::DefaultWidth,
	                                UI::ProgressBar::DefaultHeight, UI::ProgressBar::Horizontal);
	progress_->set_total(1 << 16);
	box.add(progress_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	box.add_space(8);

	// Add the wares queue
	for (uint32_t i = 0; i < dismantle_site->nr_dropout_waresqueues(); ++i) {
		subbox.add(new InputQueueDisplay(&subbox, *ibase(), *dismantle_site,
		                                 *dismantle_site->get_dropout_waresqueue(i), true, false,
		                                 priority_collapsed()),
		           UI::Box::Resizing::kFullSize);
	}
	for (uint32_t i = 0; i < dismantle_site->nr_consume_waresqueues(); ++i) {
		subbox.add(new InputQueueDisplay(&subbox, *ibase(), *dismantle_site,
		                                 *dismantle_site->get_consume_waresqueue(i), true, false,
		                                 priority_collapsed()),
		           UI::Box::Resizing::kFullSize);
	}

	box.add(&subbox, UI::Box::Resizing::kFullSize);

	get_tabs()->add("wares", g_image_cache->get(pic_tab_wares), &box, _("Building materials"));

	think();
	initialization_complete();
}

/*
===============
Make sure the window is redrawn when necessary.
===============
*/
void DismantleSiteWindow::think() {
	// BuildingWindow::think() will call die in case we are no longer in
	// existance.
	BuildingWindow::think();

	Widelands::DismantleSite* dismantle_site = dismantle_site_.get(ibase()->egbase());
	if (dismantle_site == nullptr) {
		return;
	}
	progress_->set_state(dismantle_site->get_built_per64k());
}
