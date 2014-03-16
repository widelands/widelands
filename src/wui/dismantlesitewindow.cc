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


#include "graphic/graphic.h"
#include "logic/dismantlesite.h"
#include "ui_basic/progressbar.h"
#include "ui_basic/tabpanel.h"
#include "wui/buildingwindow.h"

static const char pic_tab_wares[] = "pics/menu_tab_wares.png";

/**
 * Status window for dismantle sites.
 */
struct DismantleSite_Window : public Building_Window {
	DismantleSite_Window
		(Interactive_GameBase        & parent,
		 Widelands::DismantleSite &,
		 UI::Window *                & registry);

	virtual void think() override;

private:
	UI::Progress_Bar * m_progress;
};


DismantleSite_Window::DismantleSite_Window
	(Interactive_GameBase        & parent,
	 Widelands::DismantleSite & cs,
	 UI::Window *                & registry)
	: Building_Window(parent, cs, registry)
{
	UI::Box & box = *new UI::Box(get_tabs(), 0, 0, UI::Box::Vertical);

	// Add the progress bar
	m_progress =
		new UI::Progress_Bar
			(&box,
			 0, 0,
			 UI::Progress_Bar::DefaultWidth, UI::Progress_Bar::DefaultHeight,
			 UI::Progress_Bar::Horizontal);
	m_progress->set_total(1 << 16);
	box.add(m_progress, UI::Box::AlignCenter);

	box.add_space(8);

	// Add the wares queue
	for (uint32_t i = 0; i < cs.get_nrwaresqueues(); ++i)
		Building_Window::create_ware_queue_panel(&box, cs, cs.get_waresqueue(i), true);

	get_tabs()->add("wares", g_gr->images().get(pic_tab_wares), &box, _("Building materials"));
}


/*
===============
Make sure the window is redrawn when necessary.
===============
*/
void DismantleSite_Window::think()
{
	Building_Window::think();

	const Widelands::DismantleSite & ds =
		ref_cast<Widelands::DismantleSite, Widelands::Building>(building());

	m_progress->set_state(ds.get_built_per64k());
}


/*
===============
Create the status window describing the site.
===============
*/
void Widelands::DismantleSite::create_options_window
	(Interactive_GameBase & parent, UI::Window * & registry)
{
	new DismantleSite_Window(parent, *this, registry);
}
