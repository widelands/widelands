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

#include "buildingwindow.h"
#include "logic/militarysite.h"
#include "soldiercapacitycontrol.h"
#include "soldierlist.h"
#include "ui_basic/tabpanel.h"

using Widelands::MilitarySite;

static char const * pic_tab_military = "pics/menu_tab_military.png";

/**
 * Status window for \ref MilitarySite
 */
struct MilitarySite_Window : public Building_Window {
	MilitarySite_Window
		(Interactive_GameBase & parent,
		 MilitarySite       &,
		 UI::Window *       & registry);

	MilitarySite & militarysite() {
		return ref_cast<MilitarySite, Widelands::Building>(building());
	}

protected:
	virtual void create_capsbuttons(UI::Box * buttons);
};


MilitarySite_Window::MilitarySite_Window
	(Interactive_GameBase & parent,
	 MilitarySite       & ms,
	 UI::Window *       & registry)
:
Building_Window(parent, ms, registry)
{
	get_tabs()->add
		("soldiers", g_gr->get_picture(PicMod_Game, pic_tab_military),
		 create_soldier_list(*get_tabs(), parent, militarysite()),
		 _("Soldiers"));
}

void MilitarySite_Window::create_capsbuttons(UI::Box * buttons)
{
	Building_Window::create_capsbuttons(buttons);

	buttons->add_space(8);
	buttons->add
		(create_soldier_capacity_control(*buttons, igbase(), militarysite()),
		 UI::Box::AlignCenter);
}

/**
 * Create the  military site information window.
 */
void MilitarySite::create_options_window
	(Interactive_GameBase & plr, UI::Window * & registry)
{
	new MilitarySite_Window(plr, *this, registry);
}
