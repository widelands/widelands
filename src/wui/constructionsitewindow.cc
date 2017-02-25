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

#include "wui/buildingwindow.h"

#include "wui/inputqueuedisplay.h"

#include "graphic/graphic.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "ui_basic/progressbar.h"
#include "ui_basic/tabpanel.h"

static const char pic_tab_wares[] = "images/wui/buildings/menu_tab_wares.png";

/**
 * Status window for construction sites.
 */
struct ConstructionSiteWindow : public BuildingWindow {
	ConstructionSiteWindow(InteractiveGameBase& parent,
	                       Widelands::ConstructionSite&,
	                       UI::Window*& registry);

	void think() override;

private:
	UI::ProgressBar* progress_;
};

ConstructionSiteWindow::ConstructionSiteWindow(InteractiveGameBase& parent,
                                               Widelands::ConstructionSite& cs,
                                               UI::Window*& registry)
   : BuildingWindow(parent, cs, registry) {
	UI::Box& box = *new UI::Box(get_tabs(), 0, 0, UI::Box::Vertical);

	// Add the progress bar
	progress_ = new UI::ProgressBar(&box, 0, 0, UI::ProgressBar::DefaultWidth,
	                                UI::ProgressBar::DefaultHeight, UI::ProgressBar::Horizontal);
	progress_->set_total(1 << 16);
	box.add(progress_, UI::Align::kCenter);

	box.add_space(8);

	// Add the wares queue
	for (uint32_t i = 0; i < cs.get_nrwaresqueues(); ++i)
		box.add(
			new InputQueueDisplay(&box, 0, 0, igbase(), cs, cs.get_waresqueue(i)));

	get_tabs()->add("wares", g_gr->images().get(pic_tab_wares), &box, _("Building materials"));
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

/*
===============
Create the status window describing the construction site.
===============
*/
void Widelands::ConstructionSite::create_options_window(InteractiveGameBase& parent,
                                                        UI::Window*& registry) {
	new ConstructionSiteWindow(parent, *this, registry);
}
