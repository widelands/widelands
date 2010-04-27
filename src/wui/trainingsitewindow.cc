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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "logic/trainingsite.h"
#include "productionsitewindow.h"
#include "soldiercapacitycontrol.h"
#include "soldierlist.h"
#include "ui_basic/tabpanel.h"

using Widelands::TrainingSite;
using Widelands::atrAttack;

static char const * pic_tab_military = "pics/menu_tab_military.png";

/**
 * Status window for \ref TrainingSite
 */
struct TrainingSite_Window : public ProductionSite_Window {
	TrainingSite_Window
		(Interactive_GameBase & parent, TrainingSite &, UI::Window * & registry);

	TrainingSite & trainingsite() {
		return ref_cast<TrainingSite, Widelands::Building>(building());
	}

protected:
	virtual void create_capsbuttons(UI::Box * buttons);
};

/**
 * Create the \ref TrainingSite specific soldier list tab.
 */
TrainingSite_Window::TrainingSite_Window
	(Interactive_GameBase & parent, TrainingSite & ts, UI::Window * & registry)
:
ProductionSite_Window  (parent, ts, registry)
{
	get_tabs()->add
		("soldiers", g_gr->get_picture(PicMod_Game, pic_tab_military),
		 create_soldier_list(*get_tabs(), parent, trainingsite()),
		 _("Soldiers in training"));
}

void TrainingSite_Window::create_capsbuttons(UI::Box* buttons)
{
	ProductionSite_Window::create_capsbuttons(buttons);

	buttons->add_space(8);
	buttons->add
		(create_soldier_capacity_control(*buttons, igbase(), trainingsite()),
		 UI::Box::AlignCenter);
}

/*
===============
Create the training site information window.
===============
*/
void TrainingSite::create_options_window
	(Interactive_GameBase & plr, UI::Window * & registry)
{
	new TrainingSite_Window(plr, *this, registry);
}
